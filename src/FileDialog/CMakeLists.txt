
add_library( FileDialog
    FileDialog.cpp
    FileDialog.h

    FileHandler.cpp
    FileHandler.h
)

target_include_directories(FileDialog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(FileDialog 
    Imagehandler
)