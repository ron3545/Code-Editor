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

find_package(glfw3 REQUIRED)
# Use  cmake -S . -B build/ -DOpenGL_GL_PREFERENCE=GLVND
# to skip legacy OpenGL library
set(OpenGL_GL_PREFERENCE GLVND)
#find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(${PROJECT_NAME} RobLy_Core.cpp)

include_directories(${GLEW_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Window_Manager
    
    imgui 
    CodeEditor
    Algo
    Imagehandler
    
    glfw 
    #OpenGL
    ${GLEW_LIBRARIES}
)
