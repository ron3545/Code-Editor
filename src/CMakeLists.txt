cmake_minimum_required(VERSION 3.6)

project(ARMSIMPRO_core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_subdirectory(imgui)
add_subdirectory(ToolBar)
add_subdirectory(StatusBar)
add_subdirectory(Editor)
add_subdirectory(FileDialog)

add_executable(${PROJECT_NAME} WIN32  CodeEditor.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui 
    StatusBar
    ToolBar
    Editor
    FileDialog

    d3d11.lib 
    dxgi.lib 
    d3dcompiler.lib
    nlohmann_json::nlohmann_json
)
