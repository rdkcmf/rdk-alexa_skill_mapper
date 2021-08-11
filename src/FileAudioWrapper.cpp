/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

/**
 * @file FileAudioWrapper.cpp
 * @brief it contains the implimentaion to read the wav file data and inject
 * data in to the shared buffer of the alexa.
 */

#include <cstring>
#include <string>
#include <iostream>
#include <fstream>

#include <rapidjson/document.h>

#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>
#include <AVSCommon/Utils/Logger/Logger.h>
#include "VoiceToApps/FileAudioWrapper.h"
#include <VoiceToApps/VoiceToApps.h>

namespace alexaClientSDK {
namespace sampleApp {

///Shared Data Stream write Timeout .
static const std::chrono::milliseconds WRITING_TIMEOUT{200};

/// no of  millisecons in a second.
static constexpr unsigned int MILLISECONDS_PER_SECOND = 1000;

using avsCommon::avs::AudioInputStream;
using namespace avsCommon::utils::timing;

//  logging 
static const std::string TAG("FileAudioWrapper");

#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

static inline unsigned int numOfSampleCountPerTimeout(unsigned int rateSample) {
    return (rateSample / MILLISECONDS_PER_SECOND) * WRITING_TIMEOUT.count();
}

std::unique_ptr<FileAudioWrapper> FileAudioWrapper::create(
    std::shared_ptr<AudioInputStream> streamInput) {
    if (!streamInput) {
        ACSDK_CRITICAL(LX("Invalid stream passed to FileAudioWrapper"));
        return nullptr;
    }
    std::unique_ptr<FileAudioWrapper> fileAudioWrapper(new FileAudioWrapper(streamInput));
    if (!fileAudioWrapper->initialize()) {
        ACSDK_CRITICAL(LX("Failed to initialize fileAudioWrapper"));
        return nullptr;
    }
    return fileAudioWrapper;
}

FileAudioWrapper::FileAudioWrapper(std::shared_ptr<AudioInputStream> streamInput) :
        m_audioFileInputStream{streamInput},
        m_isFileStreaming{false},
        m_sampleCountPerTimeout{numOfSampleCountPerTimeout(AUDIOSAMPLERATE)},
        m_fileInjectionDataCounter{0} {
    m_silenceDataHolder = alexaClientSDK::avsCommon::avs::AudioInputStream::Buffer(m_sampleCountPerTimeout);
    std::fill(m_silenceDataHolder.begin(), m_silenceDataHolder.end(), 0);
}

FileAudioWrapper::~FileAudioWrapper() {
std::lock_guard<std::mutex> lock(m_mutex);
    m_fileDataTimer.stop();
}

bool FileAudioWrapper::initialize() {
    m_fileWriter = m_audioFileInputStream->createWriter(AudioInputStream::Writer::Policy::BLOCKING);
    if (!m_fileWriter) {
        ACSDK_CRITICAL(LX("Failed to create stream writer"));
        return false;
    }

    return true;
}

bool wavFileRead(std::vector<uint16_t>* audioDataHolder,const std::string& filePath) {
    if (!audioDataHolder) {
        ACSDK_ERROR(LX("readAudioWAVFileFailed").d("reason", "nullptraudioDataHolder"));
        return false;
    }

    // Opening the audio .wav file in the binary mode .
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile.good()) {
        ACSDK_ERROR(LX("readAudioWAVFileFailed").d("reason", "failed to open the file "));
        return false;
    }

    // Condition to check the size of the .wav file is greater than the .wav header
    inputFile.seekg(0, std::ios::end);
    int byteCountInFile = inputFile.tellg();

    const int audioWavFileHeaderSize = sizeof(AudioWavFileHeader);
    if (byteCountInFile <= audioWavFileHeaderSize) {
        ACSDK_ERROR(LX("readAudioWAVFileFailed").d("reason", "no of bytes read from the file is less than wav file header"));
        return false;
    }

    inputFile.seekg(0, std::ios::beg);

    // WAV file header reading .
    AudioWavFileHeader wavFileHeader;
    inputFile.read(reinterpret_cast<char*>(&wavFileHeader), audioWavFileHeaderSize);

    if (static_cast<size_t>(inputFile.gcount()) != audioWavFileHeaderSize) {
        ACSDK_ERROR(LX("readAudioiWAVFileFailed").d("reason", "header file read fail"));
        return false;
        }


    // Reading the  wav file data apart from the header.
    int numDataSamples = (byteCountInFile - audioWavFileHeaderSize) / sizeof(uint16_t);

    audioDataHolder->resize(numDataSamples, 0);
    inputFile.read(reinterpret_cast<char*>(&(audioDataHolder->at(0))), numDataSamples * sizeof(uint16_t));

    if (static_cast<size_t>(inputFile.gcount()) != (numDataSamples * sizeof(uint16_t))) {
        ACSDK_ERROR(LX("readAudioiWAVFileFailed").d("reason", "audio data read fail"));
        return false;
    }

    return true;
}


void FileAudioWrapper::write() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isFileStreaming) {
        // If audio data is empty in the file fill the buffer with silence.
        if (m_fileInjectionData.empty()) {
            ssize_t writeResultCount = m_fileWriter->write(
                static_cast<void*>(m_silenceDataHolder.data()), m_sampleCountPerTimeout, WRITING_TIMEOUT);
            if (writeResultCount <= 0) {
                ACSDK_ERROR(LX("silenceWriteFailed").d("reason", "failed to write to stream")
                                        .d("errorCode", writeResultCount));
            } else {
                ACSDK_DEBUG9(LX("writeSilenceData").d("wordsWritten", writeResultCount));
            }
        } else {
            // Inject actual wav file data.

            if (m_fileInjectionDataCounter >= m_fileInjectionData.size()) {
                ACSDK_ERROR(LX("injectAudioFailed")
                                .d("reason", "bufferOverrun")
                                .d("overrun", m_fileInjectionDataCounter - m_fileInjectionData.size()));
                     m_fileInjectionDataCounter = 0;
                
                return;
            }

            // m_fileInjectionData.size() is guaranteed to be greater than m_fileInjectDataCounter
            size_t injectionDataLeft = m_fileInjectionData.size() - m_fileInjectionDataCounter;
            size_t amountToWrite =
                (m_sampleCountPerTimeout > injectionDataLeft) ? injectionDataLeft : m_sampleCountPerTimeout;
            ssize_t writeResultCount = m_fileWriter->write(
                static_cast<void*>(m_fileInjectionData.data() + m_fileInjectionDataCounter),
                amountToWrite,
                WRITING_TIMEOUT);

            if (writeResultCount <= 0) {
                ACSDK_ERROR(LX("failed to inject audio").d("error", writeResultCount));
                resetAudioFileInjection();
            } else {
                ACSDK_DEBUG9(LX("injectAudio").d("wordsWritten", writeResultCount));
                m_fileInjectionDataCounter += writeResultCount;

                // data injection is done.
                if (m_fileInjectionDataCounter == m_fileInjectionData.size()) {
                   resetAudioFileInjection();
                }
            }
        }
    }
}

bool FileAudioWrapper::startStreamingMicrophoneData() {
        ACSDK_INFO(LX(__func__));
        skillmapper::voiceToApps inpectInputFileObj;
        std::lock_guard<std::mutex> lock{m_mutex};
        std::string file;    
        if(inpectInputFileObj.skipMerge==false){
                file=OUTPUTWAVFILE;
        }
        else{
                file=RECORDWAVFILE;
        }
        printf("%s\n",file.c_str());
        m_isFileStreaming = true;
        std::vector<uint16_t> audioData ;
        if (!wavFileRead(&audioData,file)) {
                m_fileInjectionDataCounter = 0;
                startTimer();
        }
        else {
                m_fileInjectionData = audioData;
                m_fileInjectionDataCounter = 0;
                startTimer();
        }
        return true;
}

void FileAudioWrapper::startTimer() {
    ACSDK_DEBUG5(LX(__func__));
    if (!m_fileDataTimer.isActive()) {
        m_fileDataTimer.start(
            std::chrono::milliseconds(0),
            WRITING_TIMEOUT,
            Timer::PeriodType::RELATIVE,
            Timer::getForever(),
            std::bind(&FileAudioWrapper::write, this));
    }
}

bool FileAudioWrapper::stopStreamingMicrophoneData() {
    ACSDK_INFO(LX(__func__));
    std::lock_guard<std::mutex> lock{m_mutex};
    m_isFileStreaming = false;
    resetAudioFileInjection();
    m_fileDataTimer.stop();
    return true;
}

bool FileAudioWrapper::isStreaming() {
    return m_isFileStreaming;
}

void FileAudioWrapper::resetAudioFileInjection() {
    m_fileInjectionData.clear();
    m_fileInjectionDataCounter = 0;
}



}  // namespace sampleApp
}  // namespace alexaClientSDK
