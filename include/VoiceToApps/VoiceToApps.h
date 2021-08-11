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

#ifndef ALEXA_CLIENT_SDK_VOICETOAPPS_INCLUDE_VOICETOAPPS_VOICETOAPPS_H_
#define ALEXA_CLIENT_SDK_VOICETOAPPS_INCLUDE_VOICETOAPPS_VOICETOAPPS_H_

#include <string.h>
#include <pthread.h>
#include <rapidjson/document.h>
#include "rapidjson/filereadstream.h"
#include <curl/curl.h>

#ifdef FILEAUDIO
#include <sndfile.h>
#include <core/core.h>
#include <tracing/tracing.h>
#include <websocket/websocket.h>
#include <interfaces/json/JsonData_BluetoothRemoteControl.h>
#endif

namespace skillmapper {

#ifdef FILEAUDIO
#define PREFIXWAVFILE       "/home/root/Alexa_SDK/Integration/device.wav"
#define OUTPUTWAVFILE       "/tmp/BluetoothRemoteControl/Output.wav"
#define RECORDWAVFILE       "/tmp/BluetoothRemoteControl/record.wav"
#define AUDIOSAMPLERATE     32000
#define AUDIOCHANNELS       1
#define SEARCHSTRINGOFFSET  14
#define MERGEFILECOUNT      2
#endif 

int const BUFFER_LEN = (1 << 16);
int const BUFFER_SIZE = (1 << 10);
int const PINONE = 1;
int const PINTWO = 2;
int const PINTHREE = 3;
#define RDKSERVICE_SKILLMAP_FILE       "/home/root/Alexa_SDK/Integration/AlexaRdkServiceSkillMap.json"
#define CURLPARSEFILE       "/home/root/Alexa_SDK/Integration/AlexaCurl.json"
#define SSCONTROLFILE       "/home/root/Alexa_SDK/Integration/SmartScreenControl.json"

using namespace std;

enum class VoiceSDKState {
 VTA_UNKNOWN,
 VTA_INIT,
 VTA_IDLE,
 VTA_LISTENING,
 VTA_THINKING,
 VTA_EXPECTING,
 VTA_SPEAKING
};

enum class AudioPlayerState {
 IDLE, 
 PLAYING, 
 STOPPED, 
 PAUSED, 
 BUFFER_UNDERRUN, 
 FINISHED, 
 UNKNOWN,
};

enum class OverlayState {
 INIT, 
 SHOW,
 DIMMED,
 HIDE,
};

class voiceToApps{
public:    
    voiceToApps();
    ~voiceToApps();
bool ioParse();
int  ioToggle(bool stateChange,int pin);
void handleAudioPlayerStateChangeNotification(AudioPlayerState state);
void controlOverlay(OverlayState state);
std::string overlayStateToStr(OverlayState state);

int handleSDKStateChangeNotification(VoiceSDKState state, bool handle_gui, bool audioStatePlaying);
int controlSmartScreenOverlay(VoiceSDKState state, bool audioStatePlaying);

int curlCmdSendOnRcvMsg(const string& message);
int avsDirectiveToThunderCmd(const string& message, int = 0);
#ifdef FILEAUDIO
bool mergeWavFile(const char* file1, const char*file2);
void BluetoothRemoteRPCComInit();
static bool injectAudio;
static bool skipMerge;
static bool fromExpecting;
static bool invocationMode;
#endif 
static int IO_ONE;
static int IO_TWO;
static int IO_THREE;
};

}   //namespace skillmapper

#endif  //ALEXA_CLIENT_SDK_VOICETOAPPS_INCLUDE_VOICETOAPPS_VOICETOAPPS_H_
