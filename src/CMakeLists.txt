# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_package(WPEFramework)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(${NAMESPACE}Plugins REQUIRED)

if (FILEAUDIO)
find_package(CompileSettingsDebug CONFIG REQUIRED)
endif() 

set (CMAKE_CXX_STANDARD 11)
#For AWS CPP SDK
find_package(AWSSDK REQUIRED COMPONENTS sqs)

if (FILEAUDIO)
add_library(VoiceToApps SHARED VoiceToApps.cpp FileAudioWrapper.cpp VideoSkillInterface.cpp VideoSkillJsonUtils.cpp VideoSkillConnector.cpp)
#add_library(VoiceToApps SHARED VoiceToApps.cpp FileAudioWrapper.cpp)
else()
add_library(VoiceToApps SHARED VoiceToApps.cpp VideoSkillInterface.cpp VideoSkillJsonUtils.cpp VideoSkillConnector.cpp)
#add_library(VoiceToApps SHARED VoiceToApps.cpp)
endif()
target_include_directories(VoiceToApps PUBLIC
        "${AVSCommon_INCLUDE_DIRS}"
        "${AudioResources_SOURCE_DIR}/include"
        "${VoiceToApps_SOURCE_DIR}/include")

if (FILEAUDIO)
target_link_libraries(VoiceToApps
        CompileSettingsDebug::CompileSettingsDebug
        sndfile)
endif()

target_link_libraries(VoiceToApps ${NAMESPACE}Definitions::${NAMESPACE}Definitions ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

target_link_libraries(VoiceToApps AVSCommon pthread ${AWSSDK_LINK_LIBRARIES})

if (JSONRPC_SECURITY_TOKEN)
target_link_libraries(VoiceToApps WPEFrameworkSecurityUtil)
endif()
#target_link_libraries(VoiceToApps AVSCommon pthread)

# install target
skillmapper_install()

