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

function(skillmapper_install_lib)
    SET(PKG_CONFIG_LIBS "${PKG_CONFIG_LIBS} -l${PROJECT_NAME}" CACHE INTERNAL "" FORCE)
    install(TARGETS ${PROJECT_NAME} DESTINATION "${VOICETOAPPS_LIB_INSTALL_DIR}")
endfunction()

# Function to install the target
function(skillmapper_install)
    SET(PKG_CONFIG_LIBS "${PKG_CONFIG_LIBS} -l${PROJECT_NAME}" CACHE INTERNAL "" FORCE)
    install(TARGETS ${PROJECT_NAME} DESTINATION "${VOICETOAPPS_LIB_INSTALL_DIR}")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/" DESTINATION "${VOICETOAPPS_INCLUDE_INSTALL_DIR}/include/")
endfunction()

# Function to install the target with multiple include paths
function(skillmapper_install_multiple path_list)
    SET(PKG_CONFIG_LIBS "${PKG_CONFIG_LIBS} -l${PROJECT_NAME}" CACHE INTERNAL "" FORCE)
    install(TARGETS ${PROJECT_NAME} DESTINATION "${VOICETOAPPS_LIB_INSTALL_DIR}")
    foreach(path IN LISTS path_list)
        install(DIRECTORY "${path}/" DESTINATION "${VOICETOAPPS_INCLUDE_INSTALL_DIR}/include/")
    endforeach()
endfunction()

# Set pkg_config variables
SET(PKG_CONFIG_REQUIRES "libcurl")
SET(PKG_CONFIG_LIBS         "-L\${libdir}" CACHE INTERNAL "" FORCE)
SET(PKG_CONFIG_LIBDIR       "\${prefix}/lib")
SET(PKG_CONFIG_INCLUDEDIR   "\${prefix}/include")
SET(PKG_CONFIG_CFLAGS       "-I\${includedir}")

# Set library and header files install path
SET(VOICETOAPPS_LIB_INSTALL_DIR     "${CMAKE_INSTALL_PREFIX}/lib")
SET(VOICETOAPPS_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
