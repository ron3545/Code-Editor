
add_library( FileDialog
    FileDialog.cpp
    FileDialog.h
)

target_include_directories(FileDialog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
