cmake_minimum_required(VERSION 3.6)

project(ARMSIMPRO_core LANGUAGES CXX)

#set(CMAKE_TOOLCHAIN_FILE "C:/DEV/vcpkg/scripts/buildsystems/vcpkg.cmake")
#set(CMAKE_PREFIX_PATH "C:/DEV/vcpkg/installed/x64-windows/share")  #pakage manager for C++

add_subdirectory(imgui)
add_subdirectory(CodeEditor)
add_subdirectory(ToolBar)
add_subdirectory(StatusBar)
add_subdirectory(Editor)
add_subdirectory(FileDialog)
add_subdirectory(Algorithms)
add_subdirectory(Dependencies)
add_subdirectory(ImageHandler)

add_executable(${PROJECT_NAME} WIN32  ArmSimPro.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    imgui 
    CodeEditor
    d3d11.lib 
    Algo
    mio
    Imagehandler
)

