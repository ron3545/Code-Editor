add_library( imgui "" )

file(GLOB HeadersFiles "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB SourceFiles "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

target_sources( imgui 
    PRIVATE 
        ${SourceFiles}
    PUBLIC
        ${HeadersFiles}
)

if (WIN32)
    target_include_directories( imgui
        PUBLIC 
            ${CMAKE_CURRENT_LIST_DIR}
            d3d11.lib 
    )
endif (WIN32)

include_directories (
    /usr/include/freetype2
)

target_include_directories(imgui PRIVATE 
    ${FREETYPE_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(imgui 
    ${FREETYPE_LIBRARIES} 
    ${SDL2_LIBRARIES}
)



