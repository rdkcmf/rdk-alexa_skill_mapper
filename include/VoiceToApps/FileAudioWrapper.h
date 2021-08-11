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

#ifndef ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_FILEAUDIOWRAPPER_H_
#define ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_FILEAUDIOWRAPPER_H_

#include <mutex>
#include <thread>

#include <AVSCommon/AVS/AudioInputStream.h>
#include <Audio/MicrophoneInterface.h>
#include <AVSCommon/Utils/Timing/Timer.h>

namespace alexaClientSDK {
namespace sampleApp {

// WAV file header structure
struct AudioWavFileHeader {
    uint8_t m_riffHeader[4];
    uint32_t m_chunkSize;
    uint8_t m_waveHeader[4];
    uint8_t m_fmtHeader[4];
    uint32_t m_subChunk1Size;
    uint16_t m_audioFormat;
    uint16_t m_numberOfChannels;
    uint32_t m_samplesPerSec;
    uint32_t m_bytesPerSec;
    uint16_t m_blockAlign;
    uint16_t m_bitsPerSample;
    uint8_t m_subchunk2ID[4];
    uint32_t m_subchunk2Size;
};

/// This acts as a wrapper to inject the wav file in to the alexa shareddatabuffer.
class FileAudioWrapper : public applicationUtilities::resources::audio::MicrophoneInterface {
public:
    /**
     * Creates a @c FileAudioWrapper.
     *
     * @param stream The shared data stream to write to.
     * @return A unique_ptr to a @c FileAudioWrapper if creation was successful and @c nullptr otherwise.
     */
    static std::unique_ptr<FileAudioWrapper> create(std::shared_ptr<avsCommon::avs::AudioInputStream> stream);

    /**
     * Stops streaming audio data from the WAV file.
     *
     * @return Whether the stop was successful.
     */
    bool stopStreamingMicrophoneData() override;

    /**
     * Starts streaming audio data from the WAV file.
     *
     * @return Whether the start was successful.
     */
    bool startStreamingMicrophoneData() override;

    /**
     * Whether the audio data from the WAV file is writing in to the shared buffer  .
     *
     * @return Whether the audio data from the WAV is writing in to the shared buffer.
     */
    bool isStreaming() override;

    
    void write();

    /**
     * Destructor.
     */
    ~FileAudioWrapper();

private:
    /**
     * Constructor.
     *
     * @param stream The shared data stream to write to.
     */
    FileAudioWrapper(std::shared_ptr<avsCommon::avs::AudioInputStream> stream);

    /*
     * Reset m_injectionData to empty and m_injectionDataCounter to 0.
     */
    void resetAudioFileInjection();


    /// Initializes FileAudio
    bool initialize();

    /**
     * Starts timer for writing data to the shared stream.
     */
    void startTimer();


    /// The stream of audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_audioFileInputStream;

    /// The writer that will be used to writer audio data into the sds.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Writer> m_fileWriter;

    std::mutex m_mutex;

    /**
     * Whether the WAV file is currently streaming.
     */
    bool m_isFileStreaming;

    /// no of samples exists in the timeout period.
    unsigned int m_sampleCountPerTimeout;

    //silence audio data holder
    alexaClientSDK::avsCommon::avs::AudioInputStream::Buffer m_silenceDataHolder;
    
    /// Counter for how much of current audio injection data has been injected so far.
    unsigned long m_fileInjectionDataCounter;

    /// The audio injection vector.
    std::vector<uint16_t> m_fileInjectionData;
    
    
    ///timer used to handle the file injection.
    avsCommon::utils::timing::Timer m_fileDataTimer;

};

}  // namespace sampleApp
}  // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_FILEAUDIOWRAPPER_H_
