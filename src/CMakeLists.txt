cmake_minimum_required(VERSION 3.6)

project(ARMSIMPRO_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(imgui)
add_subdirectory(Imagehandler)
add_subdirectory(ToolBar)
add_subdirectory(MenuBar)
add_subdirectory(StatusBar)

add_executable(${PROJECT_NAME} WIN32  CodeEditor.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui 
    Imagehandler
    MenuBar
    StatusBar
    ToolBar

    d3d11.lib 
    dxgi.lib 
    d3dcompiler.lib
    )