/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2021 RDK Management
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

#include "VoiceToApps/VideoSkillJsonUtils.h"

#include <string>
#include <iostream>
#include <map>
#include <thread>

#include <unistd.h>
#include <cstdlib>
#include "VoiceToApps/VoiceToApps.h"
#define SKILLMAPPER_DEBUG 1

#ifdef XR_SPEECH_AVS_SUPPORT
void (*avs_vsk_msg_handler)(const char*, unsigned long);
#endif

//Enumerated types to represent different video skill API directives
enum eVSKDirectives {
    INVALID_DIRECTIVE_NAME,
    DIRECTIVE_CHANNEL_CONTROLLER,
    DIRECTIVE_REMOTE_VIDEO_PLAYER,
    DIRECTIVE_POWER_CONTROLLER,
    DIRECTIVE_SEEK_CONTROLLER,
    DIRECTIVE_PLAYBACK_CONTROLLER,
    DIRECTIVE_SPEAKER,
    DIRECTIVE_AUTHORIZATION,
    DIRECTIVE_INPUT_CONTROLLER,
    DIRECTIVE_LAUNCHER,
    DIRECTIVE_KEYPAD_CONTROLLER,
    DIRECTIVE_EQUILIZER_CONTROLLER,
    DIRECTIVE_DUCKING_CONTROLLER,
};

// Returns the enumerated value of the directive agains the input string
eVSKDirectives mapVSKDirectives(std::string directive)
{
    static const std::map<std::string, eVSKDirectives> directiveMap
    {
      {"Alexa.ChannelController"   , DIRECTIVE_CHANNEL_CONTROLLER   },
      {"Alexa.RemoteVideoPlayer"   , DIRECTIVE_REMOTE_VIDEO_PLAYER  },
      {"Alexa.PowerController"     , DIRECTIVE_POWER_CONTROLLER     },
      {"Alexa.SeekController"      , DIRECTIVE_SEEK_CONTROLLER      },
      {"Alexa.PlaybackController"  , DIRECTIVE_PLAYBACK_CONTROLLER  },
      {"Alexa.Speaker"             , DIRECTIVE_SPEAKER              },
      {"Alexa.Authorization"       , DIRECTIVE_AUTHORIZATION        },
      {"Alexa.InputController"     , DIRECTIVE_INPUT_CONTROLLER     },
      {"Alexa.Launcher"            , DIRECTIVE_LAUNCHER             },
      {"Alexa.KeypadController"    , DIRECTIVE_KEYPAD_CONTROLLER    },
      {"Alexa.EqualizerController" , DIRECTIVE_EQUILIZER_CONTROLLER },
      {"Alexa.DuckingController"   , DIRECTIVE_DUCKING_CONTROLLER   }
    };

    auto directiveIter = directiveMap.find(directive);
    if( directiveIter != directiveMap.end() ) {
        return directiveIter->second;
    }
    return INVALID_DIRECTIVE_NAME; 
}

//Convert the VSK command to RDK command
bool mapRdkToVsk(std::string vskDirective, std::string *rdkDirective)
{
    static const std::map<std::string, std::string> vskToRdkMap
    {
      { "YouTube"   , "Launch YouTube"   },
      { "Close YouTube" , "Close YouTube" },
      { "Vimeo"     , "Vimeo"            },
      { "UP"   , std::to_string(RDKSHELL_KEY_UP) },
      { "PAGE_UP"   , std::to_string(RDKSHELL_KEY_PAGEUP) },
      { "DOWN" , std::to_string(RDKSHELL_KEY_DOWN) },
      { "PAGE_DOWN" , std::to_string(RDKSHELL_KEY_PAGEDOWN) },
      { "LEFT" , std::to_string(RDKSHELL_KEY_LEFT) },
      { "PAGE_LEFT" , std::to_string(RDKSHELL_KEY_LEFT) },
      { "RIGHT", std::to_string(RDKSHELL_KEY_RIGHT) },
      { "PAGE_RIGHT", std::to_string(RDKSHELL_KEY_RIGHT) },
      { "Play"      , std::to_string(RDKSHELL_KEY_PLAY) },
      { "SELECT"      , std::to_string(RDKSHELL_KEY_ENTER) },
      { "Pause"      , std::to_string(RDKSHELL_KEY_PAUSE) },
      { "FastForward"      , std::to_string(RDKSHELL_KEY_FASTFORWARD) },
      { "Rewind"      , std::to_string(RDKSHELL_KEY_REWIND) },
      { "Stop"      , std::to_string(RDKSHELL_KEY_BACKSPACE) },
      { "BACK"      , std::to_string(RDKSHELL_KEY_BACKSPACE) },
      { "Google Search" , "Google Search" },
      { "Google Close"  , "Google Close" },
      { "Home"          , "Main UI" },
      { "Radioline" , "Radioline" },
      { "Baeble" , "BaebleMusic" },
      { "GuessThatCity" , "GuessThatCity" },
      { "SugarBoost" , "SugarBoost" },
      { "TrueOrFalse" , "TrueOrFalse" },
      { "The Weather Network", "WeatherNetwork" },
      { "AccuWeather - Weather for Life" , "AccuWeather" },
      { "France 24 English" , "FranceTwentyFour" },
      { "Euronews" , "EuroNews" },
      { "Aljazeera" , "Aljazeera" },
      {  "Wallstreet Journal" , "WallStreetJournal" },
      { "CNN go", "CNNNews" },
      { "XUMO", "Launch XUMO" },
      { "volume up", "volume up" },
      { "volume down", "volume down" },
      { "mute", "mute" },
      { "unmute", "unmute" },
      { "show video", "show video" },
      { "hide video", "hide video" },
      { "Settings", "Bluetooth Settings"},
      { "Bluetooth Settings", "Bluetooth Settings"},
      { "Internet Settings", "Internet Settings"}
    };

    auto directiveIter = vskToRdkMap.find(vskDirective);
    if( directiveIter != vskToRdkMap.end() ) {
        *rdkDirective = directiveIter->second;
        return true;
    }
    return false;
}

//Handle the ChannelController interface
static void handleChannelController(std::string cmdName, std::string &payload)
{
    rapidjson::Document payloadDoc;

    if (!parseJSONString(payload, &payloadDoc)) {
        std::cout << __func__ <<  " failed to parse payload string" << std::endl;
        return;
    }
    rapidjson::Value::ConstMemberIterator VSK_chnIt;
    if (!findJSONNode(payloadDoc, "channel", &VSK_chnIt)) {
        std::cout << __func__ << " ERR: reason=channelTagMissing" << std::endl;
        return ;
    }

    std::string chnNo;
    if(cmdName.compare("ChangeChannel") == 0) {
        if(!retrieveValue(VSK_chnIt->value, "number", &chnNo))
            std::cout << __func__ << " ERR: reason=TagMissing [number]" << std::endl;
    }
    else std::cout << __func__ << " ERR unknown command " << cmdName.c_str() << std::endl;
    std::cout << __func__ << " INFO cmd=" << cmdName.c_str() << " number=" << chnNo.c_str() << std::endl;

    return;
}

//Handle the RemotePlaybackController Interface (not implemented)
static void handleRemotePlaybackController(std::string cmdName, std::string &payload)
{
    std::cout <<  __FILE__ << __func__ << "Not implemented" << std::endl;
    return;
}

//Handle the PowerController interface (not implemented)
static void handlePowerController(std::string cmdName, std::string &payload)
{
    std::cout <<  __FILE__ << __func__ << "Not implemented" << std::endl;
    return;
}

//Handle the SeekController interface (not implemented)
static void handleSeekController(std::string cmdName, std::string &payload)
{
    std::cout <<  __FILE__ << __func__ << "Not implemented" << std::endl;
    return;
}

//Handle the Playback conroller commands
static void handlePlaybackController(std::string cmdName, std::string &payload)
{
    std::string rdkCmd;
    if (mapRdkToVsk(cmdName, &rdkCmd) == false) {
       std::cout << __func__ << "No suitable map found for command= " << cmdName.c_str() << std::endl;
       return;
    }
    skillmapper::voiceToApps rdkSkillmapper;
    //rdkSkillmapper.avsDirectiveToThunderCmd(rdkCmd);
	rdkSkillmapper.avsDirectiveToJsonRpcCmd("SendKeystroke",rdkCmd);
   
    std::cout << __func__ << " INFO: Playback Key=" << cmdName.c_str() << "rdkCmd=" << rdkCmd.c_str() << std::endl;
    return;
}

//Handle the Speaker interface (volume +/-, Mute/Unmute)
static void handleAlexaSpeaker(std::string cmdName, std::string &payload)
{
    std::string payloadName;
    rapidjson::Document payloadDoc;

    if (!parseJSONString(payload, &payloadDoc)) {
        std::cout << __func__ <<  " failed to parse payload string" << std::endl;
        return;
    }

    skillmapper::voiceToApps rdkSkillmapper;
    if(cmdName.compare("AdjustVolume") == 0) {
        std::string volume;
        std::string volumeDef;

        if(!retrieveValue(payloadDoc, "volume", &volume)) {
            std::cout << __func__ << " ERR: reason=TagMissing [volume]" << std::endl;
        }
        if(!retrieveValue(payloadDoc, "volumeDefault", &volumeDef)) {
            std::cout << __func__ << " ERR: reason=TagMissing [volumeDefault]" << std::endl;
        }
        int absVolume=std::stoi(volume);
        if(absVolume < 0)
            rdkSkillmapper.avsDirectiveToThunderCmd("volume down", absVolume);
        else
            rdkSkillmapper.avsDirectiveToThunderCmd("volume up", absVolume);

        std::cout << __func__ <<": cmd=" << cmdName.c_str() << " volume=" << volume.c_str() << " volumeDefault=" << volumeDef.c_str() << std::endl;
    }
    else if(cmdName.compare("SetVolume") == 0) {
        std::string volume;
        if(!retrieveValue(payloadDoc, "volume", &volume)) {
            std::cout << __func__ << " ERR: reason=TagMissing [volume]" << std::endl;
        }
        int absVolume=std::stoi(volume);
        rdkSkillmapper.avsDirectiveToThunderCmd("volume set", absVolume);
        std::cout << __func__ <<": cmd=" << cmdName.c_str() << " volume=" << volume.c_str() << std::endl;
    }
    else if(cmdName.compare("SetMute") == 0) {
        std::string mute;
        if(!retrieveValue(payloadDoc, "mute", &mute)) {
            std::cout << __func__ << " ERR: reason=TagMissing [mute]" << std::endl;
        }
        else {
            if(mute.compare("true")==0){
                rdkSkillmapper.avsDirectiveToThunderCmd("mute");
            }
            else if(mute.compare("false")==0) {
                rdkSkillmapper.avsDirectiveToThunderCmd("unmute");
            }
            else {
                std::cout << __func__ << " ERR: invalid value for mute=" << mute.c_str() << std::endl;
            }
            std::cout << __func__ << ": cmd=" << cmdName.c_str() << " mute=" << mute.c_str() << std::endl;
        }
    }
    else {
            std::cout << __func__ << " ERR: No valid handler for " << cmdName.c_str() << std::endl;
    }
    return;
}

//Handle Authorization interface (not implemented)
static void handleAuthorization(std::string cmdName, std::string &payload)
{
    std::cout <<  __FILE__ << __func__ << "Not implemented" << std::endl;
    return;
}

//Handle the Input control e.g. HDMI0/1, A/V (not implemented)
static void handleInputController(std::string cmdName, std::string &payload)
{
    std::cout <<  __FILE__ << __func__ << "Not implemented" << std::endl;
    return;
}

//Handle the Application Launch events
static void handleLaunchController(std::string cmdName, std::string payload)
{
    std::string id;
    std::string payloadName;
    std::string rdkCmd;
    rapidjson::Document payloadDoc;

    if (!parseJSONString(payload, &payloadDoc)) {
        std::cout << __func__ <<  " failed to parse payload string" << std::endl;
        return;
    }

    retrieveValue(payloadDoc, "identifier", &id);
    retrieveValue(payloadDoc, "name", &payloadName);

    if (mapRdkToVsk(payloadName, &rdkCmd) == false) {
       std::cout << __func__ << "No suitable map found for command= " << payloadName.c_str() << std::endl;
       return;
    }
    std::cout << __func__ << " cmd=" << cmdName.c_str() << " id=" << id.c_str() << " payloadName=" << payloadName.c_str() << "rdkCmd=" << rdkCmd.c_str() << std::endl;
    skillmapper::voiceToApps rdkSkillmapper;
    rdkSkillmapper.avsDirectiveToThunderCmd(rdkCmd);
   
    return;
}

//Handle the keypad events
static void handleKeypadController(std::string cmdName, std::string &payload)
{
    std::string payloadName;
    std::string rdkCmd;
    rapidjson::Document payloadDoc;

    if (!parseJSONString(payload, &payloadDoc)) {
        std::cout << __func__ <<  " failed to parse payload string" << std::endl;
        return;
    }

    if(cmdName.compare("SendKeystroke") == 0) {
        std::string keyStroke;
        if(!retrieveValue(payloadDoc, "keystroke", &keyStroke))
            std::cout << __func__ << " ERR: reason=TagMissing [keystroke]" << std::endl;

        mapRdkToVsk(keyStroke, &rdkCmd);
        std::cout << __func__ <<": cmd=" << cmdName.c_str() << " key=" << keyStroke.c_str() << " rdkCmd=" << rdkCmd.c_str() << std::endl;
        skillmapper::voiceToApps rdkSkillmapper;
        //rdkSkillmapper.avsDirectiveToThunderCmd(rdkCmd);
		rdkSkillmapper.avsDirectiveToJsonRpcCmd(cmdName,rdkCmd);
    }
    return;
}

//Handle Equilizer control (not implemented)
static void handleEqController(std::string cmdName, std::string &payload)
{
    std::cout <<  __FILE__ << __func__ << "Not implemented" << std::endl;
    return;
}

//Handle Ducking controller (not implemented)
static void handleDuckingController(std::string cmdName, std::string &payload)
{
    std::cout <<  __FILE__ << __func__ << "Not implemented" << std::endl;
    return;
}

//Parses the input directive, retrieves header, namespace & payload values and handles the conditions
static int handleVSKDirective(const rapidjson::Document &document)
{

    // Get 'directive'.
    rapidjson::Value::ConstMemberIterator VSK_dIt;
    if (!findJSONNode(document, "directive", &VSK_dIt)) {
        std::cout << "parseVSKHeader ERR: reason=directiveTagMissing" << std::endl;
        return -1;
    }

    // Get 'header'
    rapidjson::Value::ConstMemberIterator VSK_hdrIt;
    if (!findJSONNode(VSK_dIt->value, "header", &VSK_hdrIt)) {
        std::cout << "parseVSKHeader ERR: reason=headerTagMissing" << std::endl;
        return -1;
    }

    // Extract 'namespace'
    std::string hdrNameSpace;
    if (!retrieveValue(VSK_hdrIt->value, "namespace", &hdrNameSpace)) {
        std::cout << "parseVSKHeader ERR: reason=namespaceTagMissing" << std::endl;
        return -1;
    }

    // Extract 'doc[header][name]'
    std::string hdrName;
    if (!retrieveValue(VSK_hdrIt->value, "name", &hdrName)) {
        std::cout << "parseVSKHeader ERR: reason=nameTagMissing" << std::endl;
        return -1;
    }
#ifdef SKILLMAPPER_DEBUG
    std::cout << "handleVSKDirective INFO: Header: Namespace=" << hdrNameSpace.c_str() << " Name=" << hdrName.c_str() << std::endl;
#endif
    // Extract 'payload'
    std::string payload;
    if (!retrieveValue(VSK_dIt->value, "payload", &payload)) {
        std::cout << __func__ << " ERR: reason=payloadTagMissing" << std::endl;
    }
    else {
#ifdef SKILLMAPPER_DEBUG
        std::cout << __func__ << " INFO: Payload=" << payload.c_str() << std::endl;
#endif
    }

    switch(mapVSKDirectives(hdrNameSpace)) {

    case DIRECTIVE_CHANNEL_CONTROLLER:
        handleChannelController(hdrName, payload);
        break;

    case DIRECTIVE_REMOTE_VIDEO_PLAYER:
        handleRemotePlaybackController(hdrName, payload);
        break;

    case DIRECTIVE_POWER_CONTROLLER:
        handlePowerController(hdrName, payload);
        break;

    case DIRECTIVE_SEEK_CONTROLLER:
        handleSeekController(hdrName, payload);
        break;

    case DIRECTIVE_PLAYBACK_CONTROLLER:
        handlePlaybackController(hdrName, payload);
        break;

    case DIRECTIVE_SPEAKER:
    {
        handleAlexaSpeaker(hdrName, payload);
    }
        break;

    case DIRECTIVE_AUTHORIZATION:
        handleAuthorization(hdrName, payload);
        break;

    case DIRECTIVE_INPUT_CONTROLLER:
        handleInputController(hdrName, payload);
        break;

    case DIRECTIVE_LAUNCHER:
    {
        handleLaunchController(hdrName, payload);
    }
        break;

    case DIRECTIVE_KEYPAD_CONTROLLER:
        handleKeypadController(hdrName, payload);
        break;

    case DIRECTIVE_EQUILIZER_CONTROLLER:
        handleEqController(hdrName, payload);

    case DIRECTIVE_DUCKING_CONTROLLER:
        handleDuckingController(hdrName, payload);
        break;

    default:
        std::cout << __func__ << " ERR: No supported namespace for event: " << hdrNameSpace.c_str() << std::endl;
 }
 return 0;
}

//Parses the input string to JSON document and handles the output directive
int processJsonBuffer(const std::string & buffer)
{
    rapidjson::Document doc;

#ifdef XR_SPEECH_AVS_SUPPORT	
	if(avs_vsk_msg_handler !=NULL)
	{
		(*avs_vsk_msg_handler)(buffer.c_str(), (unsigned long)buffer.length());
	}
#endif
	
    if(parseJSONString(buffer, &doc)==true) {
        handleVSKDirective(doc);
        return 0;
    }
    return -1;
}

void set_vsk_msg_handler(void (*vsk_msg_handler)(const char*, unsigned long))
{
#ifdef XR_SPEECH_AVS_SUPPORT
	if((vsk_msg_handler != NULL)&&(avs_vsk_msg_handler == NULL))
	{
		avs_vsk_msg_handler = vsk_msg_handler;
	}
#endif
}
