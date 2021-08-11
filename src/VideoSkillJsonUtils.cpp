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

#include <VoiceToApps/VideoSkillJsonUtils.h>

#include <string>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <cstdlib>

static void serializeJSONObjectToString(const rapidjson::Value& documentNode, std::string* value) {
    if (!documentNode.IsObject()) {
        std::cout << __func__ << " ERR, reason=invalidType, type=" << documentNode.GetType() << std::endl;
        return;
    }

    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);

    if (!documentNode.Accept(writer)) {
        std::cout << __func__ << " ERR, reason=acceptFailed" << std::endl;
        return;
    }

    *value = stringBuffer.GetString();
}

bool parseJSONString(const std::string& jsonContent, rapidjson::Document* document)
{
    if (!document) {
        std::cout << __func__ << " ERR, reason=nullDocument" << std::endl;
        return false;
    }

    document->Parse(jsonContent.c_str());

    if (document->HasParseError()) {
        std::cout << __func__ << " ERR, offset=" << document->GetErrorOffset() << "error=" << GetParseError_En(document->GetParseError()) << std::endl;
        return false;
    }
    return true;
}

bool findJSONNode(const rapidjson::Value& jsonNode, const std::string& key,rapidjson::Value::ConstMemberIterator* iteratorPtr)
{
    if (!iteratorPtr) {
        std::cout << __func__ << " ERR, reason nullIteratorPtr" << std::endl;
        return false;
    }

    auto iterator = jsonNode.FindMember(key.c_str());
    if (iterator == jsonNode.MemberEnd()) {
        std::cout << __func__ << " ERR, reason missingDirectChild, child=" << key << std::endl;
        return false;
    }
    *iteratorPtr = iterator;

    return true;
}
bool convertToValue(const rapidjson::Value& documentNode, std::string* value)
{
    if (!value) {
        std::cout << __func__ << " ERR, reason=nullValue" << std::endl;
        return false;
    }

    if (documentNode.IsString()) {
        *value = documentNode.GetString();
    } else if (documentNode.IsObject()) {
        serializeJSONObjectToString(documentNode, value);
    }
    else if (documentNode.IsInt64()) {
        *value=std::to_string(documentNode.GetInt64());
    }
    else if (documentNode.IsBool()) {
        *value = (documentNode.GetBool()==true)?"true":"false";
    }
    else {
        std::cout << __func__ << " ERR, reason=invalidType, type=" << documentNode.GetType();
        return false;
    }

    return true;
}

bool retrieveValue(const rapidjson::Value& jsonNode, const std::string& key, std::string* value) {
    if (!value) {
        std::cout << __func__ << " ERR, reason=nullValue" << std::endl;
        return false;
    }

    rapidjson::Value::ConstMemberIterator iterator;
    if (!findJSONNode(jsonNode, key, &iterator)) {
        std::cout << __func__ << " ERR, reason=keyNotFound" << std::endl;
        return false;
    }

    return convertToValue(iterator->value, value);
}

int getParsedDataFromJSON(const char *fileName, rapidjson::Document *doc)
{
    FILE * fp = fopen(fileName, "rb");
    char readFileBuffer[4096];

    if(fp==NULL)
    {
        std::cout << __func__ << " ERR: reason=fileOpenFailed" << fileName << std::endl;
        return -1;
    }

    rapidjson::FileReadStream is(fp, readFileBuffer, sizeof(readFileBuffer));
    doc->ParseStream(is);
    fclose(fp);

    return 0;
}


