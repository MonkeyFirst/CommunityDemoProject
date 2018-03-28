cmake_minimum_required (VERSION 2.8.6)

if (COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
endif ()

project (GameProject)

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)
include (UrhoCommon)

set (U3D_INC ${CMAKE_SOURCE_DIR}/../Urho3D/Build/include/Urho3D)
set (TPT_INC ${CMAKE_SOURCE_DIR}/../Urho3D/Build/include/Urho3D/ThirdParty)

include_directories (${URHO3D_INCLUDE_DIRS} ${U3D_INC} ${TPT_INC})

add_subdirectory (./src)
