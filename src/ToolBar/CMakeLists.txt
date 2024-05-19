add_library( ToolBar 
    ToolBar.cpp
)

target_include_directories( ToolBar
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        imgui::imgui
)