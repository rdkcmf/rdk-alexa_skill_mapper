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

#ifndef VOICETOAPPS_INCLUDE_VOICETOAPPS_VIDEOSKILLINTERFACE_H_
#define VOICETOAPPS_INCLUDE_VOICETOAPPS_VIDEOSKILLINTERFACE_H_

using namespace std;

void* vidSkillThreadFun(void *vargp);
int handleReceiveSQSMessage(void);
int processJsonBuffer(const std::string & buffer);
void set_vsk_msg_handler(void (*vsk_msg_handler)(const char*, unsigned long));

#endif  //VOICETOAPPS_INCLUDE_VOICETOAPPS_VIDEOSKILLINTERFACE_H_
