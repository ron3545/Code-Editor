cmake_minimum_required(VERSION 3.5)

SET(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(RobLy_Core 
        DESCRIPTION "Custome Code Editor for 6 DOF Robotic Arm"
        LANGUAGES CXX)

add_subdirectory(imgui)
add_subdirectory(WindowManager)
add_subdirectory(CodeEditor)
add_subdirectory(ToolBar)
add_subdirectory(StatusBar)
add_subdirectory(Editor)
add_subdirectory(FileDialog)
add_subdirectory(Algorithms)
add_subdirectory(ImageHandler)
add_subdirectory(TerminalEmulator)

#find_package(glfw3 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(PNG)
find_package(Freetype REQUIRED)

include_directories(SDL2Test ${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} RobLy_Core.cpp)

include_directories(${GLEW_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Window_Manager
    
    imgui 
    CodeEditor
    Algo
    Imagehandler
    
    #glfw 
    #OpenGL
    TeminalEmulator

    ${GLEW_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${SDL2_LIBRARIES}
)


include_directories (/usr/include/freetype2)