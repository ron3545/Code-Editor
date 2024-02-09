include(FetchContent)

add_library( FileDialog "" )
# set(Boost_USE_STATIC_LIBS OFF)
# set(Boost_USE_MULTITHREADED ON)  
# set(Boost_USE_STATIC_RUNTIME OFF) 
# find_package(Boost 1.82.0 COMPONENTS algorithm iostreams) 


file(GLOB HeadersFiles "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB SourceFiles "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

target_sources( FileDialog 
    PRIVATE 
        ${SourceFiles}
    PUBLIC
        ${HeadersFiles}
)

# include_directories(${Boost_INCLUDE_DIRS})

target_include_directories( FileDialog
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        imgui
        Shell32.lib
        d3d11.lib 
)

target_link_libraries(FileDialog
    lwlog::lwlog

    #${Boost_LIBRARIES}
)