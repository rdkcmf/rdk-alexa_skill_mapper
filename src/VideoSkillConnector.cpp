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

#include <aws/core/Aws.h>
#include <aws/sqs/SQSClient.h>
#include <aws/sqs/model/ReceiveMessageRequest.h>
#include <aws/sqs/model/ReceiveMessageResult.h>
#include <aws/sqs/model/DeleteMessageRequest.h>

#include <string>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <cstdlib>
#include "VoiceToApps/VoiceToApps.h"
#include "VoiceToApps/VideoSkillJsonUtils.h"
#include "VoiceToApps/VideoSkillInterface.h"

//#define SKILLMAPPER_DEBUG 1
#define SQS_RECV_MAX_MESAGES 1
#define SQS_RECV_TIMEOUT 15
#define VSK_CONFIG_JSON_PATH   "/home/root/Alexa_SDK/Integration/VSKConfig.json"

using namespace skillmapper;

//Retrieve the region part from a SQS URL - pattern (https://sqs.xx-xxxx-x)
std::string getRegionFromURL(std::string queueURL)
{
  return queueURL.substr(12,9);
}

//Receives a message from input SQS URL and deletes it from the queue, passes the JSON string for further processing.
int ReceiveSQSMessage(const Aws::String& QueueURL, Aws::String& awsRegion)
{
    //std::cout << "Goint to receive SQS message" <<std::endl;
    Aws::Client::ClientConfiguration clientCfg;
    //clientCfg.region = "eu-west-1";
    clientCfg.region = awsRegion;
    clientCfg.requestTimeoutMs = 30000; //Set timeout longer enough

    Aws::SQS::SQSClient sqs(clientCfg);

    Aws::SQS::Model::ReceiveMessageRequest recvReq;
    recvReq.SetQueueUrl(QueueURL);
    recvReq.SetMaxNumberOfMessages(SQS_RECV_MAX_MESAGES);
    recvReq.SetWaitTimeSeconds(SQS_RECV_TIMEOUT);

    auto recvResult = sqs.ReceiveMessage(recvReq);
    if (!recvResult.IsSuccess()) {
        std::cout << "Error receiving message from queue " << QueueURL << ": " << recvResult.GetError().GetMessage() << std::endl;
        return -1;
    }

    const auto& messages = recvResult.GetResult().GetMessages();
    if (messages.size() == 0) {
#ifdef SKILLMAPPER_DEBUG
        std::cout << "No messages received from queue " << QueueURL << std::endl;
#endif
        return 0;
    }

    const auto& message = messages[0];
    const Aws::String & aws_directive=message.GetBody();
    const std::string directive(aws_directive.c_str(), aws_directive.size());
#ifdef SKILLMAPPER_DEBUG
    std::cout << "Received message:" << std::endl;
    std::cout << "  MessageId: " << message.GetMessageId() << std::endl;
    std::cout << "  ReceiptHandle: " << message.GetReceiptHandle() << std::endl;
    std::cout << "  Body: " << message.GetBody() << std::endl << std::endl;
#endif
    processJsonBuffer(directive);

    Aws::SQS::Model::DeleteMessageRequest delReq;
    delReq.SetQueueUrl(QueueURL);
    delReq.SetReceiptHandle(message.GetReceiptHandle());

    auto delResult = sqs.DeleteMessage(delReq);
    if (delResult.IsSuccess()) {
#ifdef SKILLMAPPER_DEBUG
        std::cout << "Successfully deleted message " << message.GetMessageId() << " from queue " << QueueURL << std::endl;
#endif
    }
    else {
        std::cout << "Error deleting message " << message.GetMessageId() << " from queue " << QueueURL << ": " << delResult.GetError().GetMessage() << std::endl;
    }

    return messages.size();
}

// Read the QueueURL and other VSK configurations from JSON file in device.
int readVSKConfigJSON(std::string *sqsURL)
{
    rapidjson::Document doc;

    std::string configFile(VSK_CONFIG_JSON_PATH);

    if(getParsedDataFromJSON(configFile.c_str(), &doc) == 0) {
        // Get 'vsk_config'
        rapidjson::Value::ConstMemberIterator Config_dIt;
        if (!findJSONNode(doc, "vsk_config", &Config_dIt)) {
            std::cout << __func__ << " ERR: reason=TagMissing [vsk_config] " << std::endl;
            return -1;
        }
        // Get 'sqs'
        rapidjson::Value::ConstMemberIterator SQS_It;
        if (!findJSONNode(Config_dIt->value, "sqs", &SQS_It)) {
            std::cout << __func__ << " ERR: reason=TagMissing [sqs] " << std::endl;
            return -1;
        }
        // Extract 'queue_url'
        if (!retrieveValue(SQS_It->value, "queue_url", sqsURL)) {
            std::cout << __func__ << " ERR: reason=TagMissing [queue_url]" << std::endl;
            return -1;
        }
    }
    else {
        std::cout << __func__ << " ERR: Unable to read configuration file : " << configFile << std::endl;
        return -1;
    }
    return 0;
}

// Initialize the AWS attributes and invoke the API to start receiving message from SQS
int handleReceiveSQSMessage(void)
{
    std::string sqsURL;
    std::string sqsRegion;
    Aws::SDKOptions sdkOpts;

    if(readVSKConfigJSON(&sqsURL)<0) {
        std::cout << __func__ << " ERR: failed to get Queue URL" << std::endl;
        return -1;
    }
    sqsRegion=getRegionFromURL(sqsURL);
    Aws::InitAPI(sdkOpts);
    {
        
        Aws::String awsRegion(sqsRegion.c_str(), sqsRegion.size());
        ReceiveSQSMessage(Aws::String(sqsURL.c_str(), sqsURL.size()), awsRegion);
    }
    Aws::ShutdownAPI(sdkOpts);
    return 0;
}

#if 0
void* vidSkillThreadFun(void *vargp){
    while(true){
        handleReceiveSQSMessage();
//        pthread_yield();
    }
//    pthread_exit(NULL);
    return NULL;
}

void createSQSReceiveTask(void)
{
    pthread_t tid;
    pthread_create(&tid, NULL, vidSkillThreadFun, NULL);
}
#endif
