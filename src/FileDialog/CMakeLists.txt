add_library( FileDialog "" )

file(GLOB HeadersFiles "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB SourceFiles "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

target_sources( FileDialog 
    PRIVATE 
        ${SourceFiles}
    PUBLIC
        ${HeadersFiles}
)

target_include_directories( FileDialog
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        imgui
        Shell32.lib
        d3d11.lib 
)

target_link_libraries(
    FileDialog
    lwlog::lwlog
)