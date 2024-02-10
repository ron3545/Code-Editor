
add_library( CodeEditor
    Editor.cpp
)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_include_directories( CodeEditor
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        imgui
        
        Shell32.lib
        d3d11.lib 
)

target_link_libraries(CodeEditor
    
    #${Boost_LIBRARIES}
    StatusBar
    ToolBar
    Editor
    FileDialog
    nlohmann_json::nlohmann_json
)