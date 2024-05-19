
add_library( CodeEditor
    CodeEditor.cpp
    AppLog.hpp
)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

# # https://github.com/ChristianPanov/lwlog/issues/19
# FetchContent_Declare(lwlog
#   GIT_REPOSITORY https://github.com/ChristianPanov/lwlog
#   GIT_TAG aed908843a23487ee31aba25318aaa73b43d1d18
# )

# FetchContent_GetProperties(lwlog)
# if(NOT lwlog_POPULATED)
#   FetchContent_Populate(lwlog)
#   add_subdirectory(${lwlog_SOURCE_DIR} ${lwlog_BINARY_DIR} EXCLUDE_FROM_ALL)
# endif()

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)


target_link_libraries(CodeEditor
    
    #${Boost_LIBRARIES}
    StatusBar
    ToolBar
    Editor
    FileDialog
    nlohmann_json::nlohmann_json

    imgui
)