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
 * @file VoiceToApps.cpp
 * @brief it consists to methods to launch application using  curl commands,
 * build remote communication and collecting the bluetooth voice farmes from
 * the bluetooth control plugin , wav file handling and led handling .
 */

#include "VoiceToApps/VoiceToApps.h"
#include<thread>
#include <iostream>

using namespace rapidjson;
using namespace skillmapper; 
using namespace std;
#ifdef FILEAUDIO
using namespace WPEFramework ;
using namespace JsonData::BluetoothRemoteControl;
using namespace skillmapper;

bool voiceToApps::injectAudio=false;
bool voiceToApps::skipMerge=true;
bool voiceToApps::invocationMode=false;
bool voiceToApps::fromExpecting=false;

int frameSkipCount;
SNDFILE *sndFile = NULL;
#endif //FILEAUDIO

int voiceToApps::IO_ONE=0;
int voiceToApps::IO_TWO=0;
int voiceToApps::IO_THREE=0;

voiceToApps::voiceToApps()
{
}

voiceToApps::~voiceToApps()
{
}

bool voiceToApps::ioParse()
{
#ifdef RDK_FRONTPANEL_SUPPORT
      
        char readFileBuffer[BUFFER_LEN];
        FILE * fp = fopen(CURLPARSEFILE, "rb");
        if(fp==NULL)
        {
                printf("failed to open the fiel %s\n",CURLPARSEFILE);
                return false;
        }
        FileReadStream is(fp, readFileBuffer, sizeof(readFileBuffer));

        Document doc;
        doc.ParseStream(is);
        const rapidjson::Value & attributes = doc["attributes"];
        assert(attributes.IsArray());
        for (rapidjson::Value::ConstValueIterator itr = attributes.Begin(); itr != attributes.End(); ++itr) {
                const rapidjson::Value & attribute = * itr;
                if(!strcmp("PinOne",attribute.GetString())){
                    this->IO_ONE=std::atoi(doc[attribute.GetString()][0].GetString());
                }
                else if(!strcmp("PinTwo",attribute.GetString())){
                    this->IO_TWO=std::atoi(doc[attribute.GetString()][0].GetString());
                }
                else if(!strcmp("PinThree",attribute.GetString())){
                    this->IO_THREE=std::atoi(doc[attribute.GetString()][0].GetString());
                }
        }
        fclose(fp);
#endif
      return true;
}

std::string VoiceSDKStateToStr(VoiceSDKState& state)
{
        if     (state == VoiceSDKState::VTA_IDLE)      return "VTA_IDLE";
        else if(state == VoiceSDKState::VTA_INIT)      return "VTA_INIT";
        else if(state == VoiceSDKState::VTA_LISTENING) return "VTA_LISTENING";
        else if(state == VoiceSDKState::VTA_THINKING)  return "VTA_THINKING";
        else if(state == VoiceSDKState::VTA_EXPECTING) return "VTA_EXPECTING";
        else if(state == VoiceSDKState::VTA_SPEAKING)  return "VTA_SPEAKING";

        return "VTA_UNKNOWN";
}

std::string voiceToApps::overlayStateToStr(OverlayState state){
	std::string retStr;
	if      ( state ==  OverlayState::INIT) retStr = "start-overlay";
	else if (state == OverlayState::SHOW)   retStr = "show-overlay";
	else if (state == OverlayState::DIMMED) retStr = "dimm-overlay";
	else if ( state == OverlayState::HIDE)  retStr = "hide-overlay";
	return retStr;
}

void voiceToApps::controlOverlay(OverlayState state){
#ifdef SMARTSCREEN_SUPPORT
       char readFileBuffer[BUFFER_LEN];
       char * curlData = new char[BUFFER_SIZE];
       CURL * curl=NULL;
       CURLcode res;
       struct curl_slist * headers = NULL;

       std::string ssState = overlayStateToStr(state);
       cout << __func__ << "Calling Overlay with state: " << ssState << std::endl;
	   
       FILE * fp = fopen(SSCONTROLFILE, "rb");
       if(fp==NULL) {
	   printf("failed to open the file %s\n",SSCONTROLFILE);
	   return;
       }
       FileReadStream is(fp, readFileBuffer, sizeof(readFileBuffer));
       fclose(fp);

       Document doc;
       doc.ParseStream(is);

       curl = curl_easy_init();
       if(curl==NULL){
           printf("curl init failed\n");
           return;
       }

       headers = curl_slist_append(headers, "Content-Type: application/json");
       curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
       curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9998/jsonrpc");

       const rapidjson::Value& att = doc[ssState.c_str()];
       assert(att.IsArray());
       for (rapidjson::Value::ConstValueIterator itr1 = att.Begin(); itr1 != att.End(); ++itr1) {
           const rapidjson::Value& at = *itr1;
           sprintf(curlData, "%s", at.GetString());
           fprintf(stderr, "%s : JSON RPC Command: %s\n", __func__, curlData);
           curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlData);
           res = curl_easy_perform(curl);
           /* Check for errors */
           if (res != CURLE_OK){
               fprintf(stderr, "curl_easy_perform() failed: %s\n",
               curl_easy_strerror(res));
               return;
           }
           this_thread::sleep_for(chrono::milliseconds(100) );
       }

       curl_easy_cleanup(curl);
       delete[] curlData;
#endif	
}

int voiceToApps::handleSDKStateChangeNotification(VoiceSDKState state, bool handle_gui,  bool audioStatePlaying)
{
	cout << __func__ << " Handle state change events .... " << VoiceSDKStateToStr(state) << ", audioState "<< audioStatePlaying << std::endl;
        if(state == VoiceSDKState::VTA_IDLE) {
            voiceToApps::ioToggle(false,skillmapper::PINONE);
            voiceToApps::ioToggle(false,skillmapper::PINTWO);
        }
        else if(state == VoiceSDKState::VTA_LISTENING) {
            voiceToApps::ioToggle(true,skillmapper::PINONE);
            voiceToApps::ioToggle(false,skillmapper::PINTWO);
        }
        else if(state == VoiceSDKState::VTA_THINKING) {
            voiceToApps::ioToggle(false,skillmapper::PINONE);
            voiceToApps::ioToggle(true,skillmapper::PINTWO);
        }
        else if(state == VoiceSDKState::VTA_EXPECTING) {
        }
        else if(state == VoiceSDKState::VTA_SPEAKING) {
            voiceToApps::ioToggle(true,skillmapper::PINONE);
            voiceToApps::ioToggle(true,skillmapper::PINTWO);
        }
        if(handle_gui==true)
            voiceToApps::controlSmartScreenOverlay(state,audioStatePlaying);

        return 0;
}

void voiceToApps::handleAudioPlayerStateChangeNotification(AudioPlayerState state){
	if ( state == AudioPlayerState::STOPPED || state == AudioPlayerState::FINISHED ){
		// hide overlay
		controlOverlay(OverlayState::HIDE);
	}
	else if ( state == AudioPlayerState::PLAYING ){
		// show overlay
		controlOverlay(OverlayState::SHOW);
	}
}


int voiceToApps::controlSmartScreenOverlay(VoiceSDKState state,  bool audioStatePlaying)
{
	
#ifdef SMARTSCREEN_SUPPORT
		OverlayState oState;
       if(state == VoiceSDKState::VTA_INIT) {
          oState = OverlayState::INIT;
       }
       else if(state == VoiceSDKState::VTA_IDLE && ! audioStatePlaying) {
	  oState = OverlayState::HIDE;
       }
       else if(state == VoiceSDKState::VTA_SPEAKING) {
          oState = OverlayState::SHOW;
       }
       else {
          oState = OverlayState::DIMMED;
       }

       controlOverlay(oState);
#endif
       return 0;
}

int voiceToApps::ioToggle(bool stateChange,int pin)
{
#ifdef RDK_FRONTPANEL_SUPPORT
        char * curlData = new char[BUFFER_SIZE];
        CURL * curl=NULL;
        CURLcode res;
        struct curl_slist * headers = NULL;
        curl = curl_easy_init();

if (curl) {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9998/jsonrpc");
        switch(pin)
        {
            case PINONE:
                if(stateChange==true)
                 sprintf(curlData, "{\"jsonrpc\": \"2.0\", \"id\": 1234567890, \"method\": \"IOConnector.1.pin@%d\",\"params\": 0}",this->IO_ONE );
                else
                 sprintf(curlData, "{\"jsonrpc\": \"2.0\", \"id\": 1234567890, \"method\": \"IOConnector.1.pin@%d\",\"params\": 1}",this->IO_ONE );
            break;
            case PINTWO:
                if(stateChange==true)
                 sprintf(curlData, "{\"jsonrpc\": \"2.0\", \"id\": 1234567890, \"method\": \"IOConnector.1.pin@%d\",\"params\": 0}",this->IO_TWO );
                else
                 sprintf(curlData, "{\"jsonrpc\": \"2.0\", \"id\": 1234567890, \"method\": \"IOConnector.1.pin@%d\",\"params\": 1}",this->IO_TWO );
            break;
            case PINTHREE:
                if(stateChange==true)
                 sprintf(curlData, "{\"jsonrpc\": \"2.0\", \"id\": 1234567890, \"method\": \"IOConnector.1.pin@%d\",\"params\": 0}",this->IO_THREE );
                else
                 sprintf(curlData, "{\"jsonrpc\": \"2.0\", \"id\": 1234567890, \"method\": \"IOConnector.1.pin@%d\",\"params\": 1}",this->IO_THREE );
            break;
        }
        //fprintf(stderr, "VoiceToApps IOConnector command %s\n", curlData);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlData);
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                curl_easy_strerror(res));
                return 1;
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
        memset(curlData, '\0', BUFFER_SIZE);
}

   delete[] curlData;
#endif
   return 0;
}

int voiceToApps::avsDirectiveToThunderCmd(const string& message, int iArg)
{
        int findResult = 0;
        char * messBuf = new char[message.length() + 1];
        char readFileBuffer[BUFFER_LEN];
        char * curlData = new char[BUFFER_SIZE];
        CURL * curl=NULL;
        CURLcode res;
        struct curl_slist * headers = NULL;

        fprintf(stderr, "VoiceToApps Entering %s message=%s\n",__func__, message.c_str());

        curl = curl_easy_init();
        if(curl==NULL){
            printf("curl init failed\n");
            return 0;
        }

        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9998/jsonrpc");


        std::strcpy(messBuf, message.c_str());

#ifdef WITH_RDKSERVICES
        char *fileName = RDKSERVICE_SKILLMAP_FILE;
#else
        char *fileName = CURLPARSEFILE;
#endif
        cout << __func__ << "Getting skill map from: " << fileName << std::endl;
        FILE * fp = fopen(fileName, "rb");
        if(fp==NULL)
        {
                printf("failed to open the file %s\n",fileName);
                return 0;
        }
        FileReadStream is(fp, readFileBuffer, sizeof(readFileBuffer));

        Document doc;
        doc.ParseStream(is);
        const rapidjson::Value & attributes = doc["attributes"];

        assert(attributes.IsArray());
        for (rapidjson::Value::ConstValueIterator itr = attributes.Begin(); itr != attributes.End(); ++itr) {
                const rapidjson::Value & attribute = * itr;
                findResult = message.find(attribute.GetString());
                if (findResult >= 0) {
                    const rapidjson::Value& att = doc[attribute.GetString()];
                    assert(att.IsArray());
                        if(0) {
                        }
                        else{
                            for (rapidjson::Value::ConstValueIterator itr1 = att.Begin(); itr1 != att.End(); ++itr1) {
                                const rapidjson::Value& at = *itr1;
                                sprintf(curlData, "%s", at.GetString());
                                fprintf(stderr, "%s : JSON RPC Command: %s\n", __func__, curlData);
                                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlData);
                                res = curl_easy_perform(curl);
                                /* Check for errors */
                                   if (res != CURLE_OK){
                                        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                        curl_easy_strerror(res));
                                        return 0;
                                    }
                                this_thread::sleep_for(chrono::milliseconds(100) );
                                }
                        }
                }

        }

    fclose(fp);
    curl_easy_cleanup(curl);
    delete[] curlData;
    delete[] messBuf;
    return 0;
}

int voiceToApps::curlCmdSendOnRcvMsg(const string& message)
{
        int findResult = 0;
        char * messBuf = new char[message.length() + 1];
        char * searchString = new char[BUFFER_SIZE];
        char readFileBuffer[BUFFER_LEN];
        char * curlData = new char[BUFFER_SIZE];
        CURL * curl=NULL;
        CURLcode res;
        struct curl_slist * headers = NULL;

        //fprintf(stderr, "VoiceToApps Entering %s message=%s\n",__func__, message.c_str());

        curl = curl_easy_init();
        if(curl==NULL){
            printf("curl init failed\n");
            return 0;
        }

        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9998/jsonrpc");


        std::strcpy(messBuf, message.c_str());
#ifdef WITH_RDKSERVICES
        char *fileName = RDKSERVICE_SKILLMAP_FILE;
#else
        char *fileName = CURLPARSEFILE;
#endif

        cout << __func__ << "Getting skill map from: " << fileName << std::endl;
        FILE * fp = fopen(fileName, "rb");
        if(fp==NULL)
        {
                printf("failed to open the file %s\n",fileName);
                return 0;
        }
        FileReadStream is(fp, readFileBuffer, sizeof(readFileBuffer));

        Document doc;
        doc.ParseStream(is);
        const rapidjson::Value & attributes = doc["attributes"];

        assert(attributes.IsArray());
        for (rapidjson::Value::ConstValueIterator itr = attributes.Begin(); itr != attributes.End(); ++itr) {
                const rapidjson::Value & attribute = * itr;
                findResult = message.find(attribute.GetString());
                if (findResult >= 0) {
                    const rapidjson::Value& att = doc[attribute.GetString()];
                    assert(att.IsArray());
                        if (!strcmp(attribute.GetString(), "Google Search")) {
                            
                            for (rapidjson::Value::ConstValueIterator itr1 = att.Begin(); itr1 != att.End(); ++itr1) {
                                const rapidjson::Value& at = *itr1;
                                if(strstr(at.GetString(),"search")){
                                    strcpy(searchString,std::strtok(messBuf + findResult + strlen("Google Search")+1, "\""));
                                    sprintf(curlData, "%s%s\"}", at.GetString(), searchString);
                                }
                                else
                                    sprintf(curlData, "%s", at.GetString());
                                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlData);
                                res = curl_easy_perform(curl);
                                /* Check for errors */
                                if (res != CURLE_OK){
                                    fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                    curl_easy_strerror(res));
                                    return 0;
                                }
                                memset(curlData, '\0', BUFFER_SIZE);
                                this_thread::sleep_for(chrono::milliseconds(100) );
                            }
                        }
                        if (!strcmp(attribute.GetString(), "YouTube Search")) {
                            for (rapidjson::Value::ConstValueIterator itr1 = att.Begin(); itr1 != att.End(); ++itr1) {
                                const rapidjson::Value& at = *itr1;
                                if(strstr(at.GetString(),"query")){
                                    strcpy(searchString,std::strtok(messBuf + findResult + strlen("YouTube Search"), "\""));
                                    sprintf(curlData, "%s%s\"}", at.GetString(), searchString);
                                }
                                else
                                    sprintf(curlData, "%s", at.GetString());
                                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlData);
                                res = curl_easy_perform(curl);
                                /* Check for errors */
                                if (res != CURLE_OK){
                                    fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                    curl_easy_strerror(res));
                                    return 0;
                                }
                                memset(curlData, '\0', BUFFER_SIZE);
                                this_thread::sleep_for(chrono::milliseconds(100) );
                            }
                        }
#ifdef FILEAUDIO
                        else if(!strcmp(attribute.GetString(), "skill mode enable")){
                                voiceToApps::skipMerge =false;
                                voiceToApps::invocationMode =true;
                        } 
                        else if(!strcmp(attribute.GetString(), "skill mode disable")){
                                voiceToApps::skipMerge =true;
                                voiceToApps::invocationMode =false;
                        } 
#endif 
                        else{
                            for (rapidjson::Value::ConstValueIterator itr1 = att.Begin(); itr1 != att.End(); ++itr1) {
                                const rapidjson::Value& at = *itr1;
                                sprintf(curlData, "%s", at.GetString());
                                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlData);
                                res = curl_easy_perform(curl);
                                /* Check for errors */
                                   if (res != CURLE_OK){
                                        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                        curl_easy_strerror(res));
                                        return 0;
                                    }
                                this_thread::sleep_for(chrono::milliseconds(100) );
                                }
                        }

                }

        }

    fclose(fp);
    curl_easy_cleanup(curl);
    delete[] curlData;
    delete[] messBuf;
    delete[] searchString;
    return 0;
}


#ifdef FILEAUDIO

static void concatWavData(SNDFILE *hOutfile, SNDFILE *hInfile, SF_INFO *info){   
	int rc=-1, fmt;
	if(BUFFER_LEN / info->channels > 0) {
		sf_seek (hOutfile, 0, SEEK_END) ;

		fmt = (info->format & SF_FORMAT_SUBMASK);
		if ( (fmt == SF_FORMAT_DOUBLE) || (fmt == SF_FORMAT_FLOAT) ) {
			static double  flt_data [BUFFER_LEN] ;

			do {
				rc = sf_readf_double(hInfile, flt_data, (BUFFER_LEN / info->channels) ) ;
				sf_writef_double(hOutfile, flt_data, rc) ;
			}while (rc > 0);
		}
		else {
			static int  int_data [BUFFER_LEN] ;

			do {
				rc = sf_readf_int (hInfile, int_data, (BUFFER_LEN / info->channels) ) ;
				sf_writef_int (hOutfile, int_data, rc) ;
			}while (rc > 0);
		}
	}
	return;
}

bool voiceToApps:: mergeWavFile(const char* inWavFile1, const char*inWavFile2){
	SNDFILE     *hOutFile, **hInFile ;
	SF_INFO     outFileInfo, inFileInfo ;
	int         k ;
	fprintf(stderr, "%s: files  %s ------%s \n",__func__, inWavFile1, inWavFile2);

	if ((hInFile =(SNDFILE**) calloc (MERGEFILECOUNT, sizeof (SNDFILE*))) == NULL){   
		fprintf(stderr, "%s: Malloc failed \n",__func__);
		return false ;
	}

	memset (&inFileInfo, 0, sizeof (inFileInfo)) ;

	if ((hInFile[0] = sf_open (inWavFile1, SFM_READ, &inFileInfo)) == NULL){   
		fprintf (stderr, "%s: Failed to open file '%s'.\n",__func__, inWavFile1);
		return false ;
	}

	outFileInfo = inFileInfo ;

	if ((hInFile[1] = sf_open (inWavFile2, SFM_READ, &inFileInfo)) == NULL){   
		fprintf (stderr, "%s: Failed to open file '%s'.\n",__func__, inWavFile2);
		return false ;
	}

	if (inFileInfo.channels != outFileInfo.channels){   
		fprintf (stderr, "%s: Channel Mismatch - '%s' has %d channels (should have %d)\n", __func__, inWavFile2, inFileInfo.channels, outFileInfo.channels) ;
		return false ;
	}

	if ((hOutFile = sf_open (OUTPUTWAVFILE, SFM_WRITE, &outFileInfo)) == NULL){   
		fprintf (stderr, "%s: Failed to open file '%s'.\n",__func__, OUTPUTWAVFILE);
		puts (sf_strerror (NULL)) ;
		return false ;
	}
	for (k = 0 ; k < MERGEFILECOUNT ; k++){   
		concatWavData(hOutFile, hInFile[k], &outFileInfo) ;
		sf_close (hInFile[k]) ;
	}

	sf_close (hOutFile) ;
	free (hInFile) ;

	return true ;
}


namespace Handlers {
        static void audioTransmissionCB(const AudiotransmissionParamsData& parameters){
                string codec= "pcm";
                voiceToApps fileWriteFlag;
                if(!codec.compare(parameters.Profile.Value().c_str())){
                        SF_INFO info;
                        frameSkipCount=20;
                        info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
                        info.channels = AUDIOCHANNELS;
                        info.samplerate = AUDIOSAMPLERATE;
                        sndFile = sf_open(RECORDWAVFILE, SFM_WRITE, &info);
                        if (sndFile == NULL) {
                                fprintf(stderr, "Error opening sound file '%s': %s\n", RECORDWAVFILE, sf_strerror(sndFile));
                        }
                }
                else{
                        sf_write_sync(sndFile);
                        sf_close(sndFile);
                        fileWriteFlag.injectAudio = true;
                }

        }
        static void audioFrameCB(const AudioframeParamsData& parameters){
                uint8_t rawDataBuffer[BUFFER_SIZE];
                uint16_t rawDataBufferLength = BUFFER_SIZE;
                short *bufShortPtr = NULL; 
                uint16_t shortBufLen = 0;
                /* skiping the intial farme to skip the silence */
                if(frameSkipCount==0)
                {
                    Core::FromString(parameters.Data.Value(),rawDataBuffer,rawDataBufferLength,"=");
                    bufShortPtr = (short*)rawDataBuffer;
                    shortBufLen = rawDataBufferLength/sizeof(short);
                    sf_write_short(sndFile,bufShortPtr, shortBufLen);
                }
                else
                    frameSkipCount--;
        }

}
void* blueThreadFun(void *vargp){
        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:80")));
        JSONRPC::LinkType<Core::JSON::IElement> remoteObject(_T("BluetoothRemoteControl.2"), _T("client.events.88"));

        if (remoteObject.Subscribe<AudiotransmissionParamsData>(1000, _T("audiotransmission"), &Handlers::audioTransmissionCB) == Core::ERROR_NONE) {
                printf("Bluetooth Audio Tramission Subscription Successful\n");
        } else {
                printf("Failed To Subscribe The Bluetooth Audio Tramission\n");
        }
        if (remoteObject.Subscribe<AudioframeParamsData>(1000, _T("audioframe"), &Handlers::audioFrameCB) == Core::ERROR_NONE) {
                printf("Bluetooth Audio Frames Subscription Successful\n");
        } else {
                printf("Failed To Subscribe The Bluetooth Audio Frames\n");
        }
        while(true)
                pthread_yield();

        pthread_exit(NULL);

}


void voiceToApps::BluetoothRemoteRPCComInit(){

        pthread_t tid;
        pthread_create(&tid, NULL, blueThreadFun, NULL);
}

#endif //FILEAUDIO
