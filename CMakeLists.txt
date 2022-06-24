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

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(VoiceToApps LANGUAGES CXX)

set(SKILLMAPPER_SMARTSCREEN_SUPPORT ON CACHE BOOL "Enable the Smart screen control functions in skillmapper")
set(SKILLMAPPER_FRONTPANEL_SUPPORT  ON CACHE BOOL "Enable the FrontPanel notification in skillmapper")
set(SKILLMAPPER_WITH_RDKSERVICE     ON CACHE BOOL "Use RDK Service plugins for skill execution")
set(SKILLMAPPER_XR_SPEECH_AVS_SUPPORT OFF CACHE BOOL "Enable the xr-speech-avs support")
if (FILEAUDIO)
    set(VALID TRUE)
    add_definitions("-DFILEAUDIO")
endif()

if (SKILLMAPPER_SMARTSCREEN_SUPPORT)
    set(VALID TRUE)
    add_definitions(-DSMARTSCREEN_SUPPORT)
endif()

if (SKILLMAPPER_FRONTPANEL_SUPPORT)
    set(VALID TRUE)
    add_definitions(-DRDK_FRONTPANEL_SUPPORT)
endif()

if (SKILLMAPPER_RDKSERVICE_SUPPORT)
    set(VALID TRUE)
    add_definitions(-DWITH_RDKSERVICES)
endif()

if (SKILLMAPPER_XR_SPEECH_AVS_SUPPORT)
    set(VALID TRUE)
    add_definitions(-DXR_SPEECH_AVS_SUPPORT)
endif()

include(PrepareInstallTarget.cmake)

add_subdirectory("src")
