add_library( Editor 
    CmdPanel.cpp
    TextEditor.cpp
    Editor.cpp
    Async_Wrapper.hpp
)

target_include_directories(Editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    imgui
)

target_link_libraries(Editor
    Algo
)
