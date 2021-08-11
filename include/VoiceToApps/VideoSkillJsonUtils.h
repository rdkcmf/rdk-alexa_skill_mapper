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
#ifndef VOICETOAPPS_INCLUDE_VOICETOAPPS_VIDEOSKILLJSONUTILS_H_
#define VOICETOAPPS_INCLUDE_VOICETOAPPS_VIDEOSKILLJSONUTILS_H_

#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <string>
#include <iostream>

bool parseJSONString(const std::string& jsonContent, rapidjson::Document* document);
bool findJSONNode(const rapidjson::Value& jsonNode, const std::string& key,rapidjson::Value::ConstMemberIterator* iteratorPtr);
bool retrieveValue(const rapidjson::Value& jsonNode, const std::string& key, std::string* value);
int getParsedDataFromJSON(const char *fileName, rapidjson::Document *doc);

#endif /*VOICETOAPPS_INCLUDE_VOICETOAPPS_VIDEOSKILLJSONUTILS_H_*/
