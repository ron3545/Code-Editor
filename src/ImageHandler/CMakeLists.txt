add_library( Imagehandler 
    ImageHandler.cpp
    ImageHandler.h
)

target_include_directories( Imagehandler
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        d3d11.lib 
)