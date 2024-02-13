cmake_minimum_required(VERSION 3.11)

project(ARMSIMPRO_core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_TOOLCHAIN_FILE "C:/DEV/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_PREFIX_PATH "C:/DEV/vcpkg/installed/x64-windows/share")  #pakage manager for C++

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

# https://github.com/ChristianPanov/lwlog/issues/19
FetchContent_Declare(lwlog
  GIT_REPOSITORY https://github.com/ChristianPanov/lwlog
  GIT_TAG aed908843a23487ee31aba25318aaa73b43d1d18
)

FetchContent_GetProperties(lwlog)
if(NOT lwlog_POPULATED)
  FetchContent_Populate(lwlog)
  add_subdirectory(${lwlog_SOURCE_DIR} ${lwlog_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_subdirectory(imgui)
add_subdirectory(CodeEditor)
add_subdirectory(ToolBar)
add_subdirectory(StatusBar)
add_subdirectory(Editor)
add_subdirectory(FileDialog)

add_executable(${PROJECT_NAME} WIN32  ArmSimPro.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    imgui 
    CodeEditor
    lwlog::lwlog
    d3d11.lib 
)
