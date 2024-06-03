add_library(Window_Manager
    WindowManager.cpp
)

include_directories(${GLEW_INCLUDE_DIRS})

target_link_libraries(Window_Manager PRIVATE 
    imgui 
    CodeEditor
    Algo
    Imagehandler
    
    #glfw 
    OpenGL
    ${GLEW_LIBRARIES}
    ${SDL2_LIBRARIES}
)