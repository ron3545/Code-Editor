add_library( MenuBar "" )

file(GLOB HeadersFiles "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB SourceFiles "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

target_sources( MenuBar 
    PRIVATE 
        ${SourceFiles}
    PUBLIC
        ${HeadersFiles}
)

target_include_directories( MenuBar
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        imgui

        d3d11.lib 
)