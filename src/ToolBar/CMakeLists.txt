add_library( ToolBar "" )

file(GLOB HeadersFiles "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB SourceFiles "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

target_sources( ToolBar 
    PRIVATE 
        ${SourceFiles}
    PUBLIC
        ${HeadersFiles}
)

target_include_directories( ToolBar
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        imgui

        d3d11.lib 
)