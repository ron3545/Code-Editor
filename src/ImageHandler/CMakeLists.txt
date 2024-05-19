add_library( Imagehandler 
    ImageHandler.cpp
    ImageHandler.h
)

target_include_directories( Imagehandler
PUBLIC 
    imgui
)