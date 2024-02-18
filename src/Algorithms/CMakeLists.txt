
add_library( Algo
    Search.cpp
)

target_include_directories( Algo
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        imgui
)

target_link_libraries(Algo
    mio
)