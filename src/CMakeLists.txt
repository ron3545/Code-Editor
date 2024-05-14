cmake_minimum_required(VERSION 3.14)

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

include(FetchContent)
FetchContent_Declare(LoguruGitRepo
	GIT_REPOSITORY "https://github.com/emilk/loguru" # can be a filesystem path
	GIT_TAG        "master"
)
# set any loguru compile-time flags before calling MakeAvailable()
set(LOGURU_WITH_STREAMS TRUE)
FetchContent_MakeAvailable(LoguruGitRepo) # defines target 'loguru::loguru'

add_executable(${PROJECT_NAME} WIN32  RoboMaster_Core.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui 
    CodeEditor
    d3d11.lib 
    Algo
    mio
    Imagehandler
    user32.lib
    loguru::loguru
)

