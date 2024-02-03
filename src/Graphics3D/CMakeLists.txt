#add_library( GraphicsHandler "" )
#
#file(GLOB HeadersFiles "${CMAKE_CURRENT_LIST_DIR}/*.h")
#file(GLOB SourceFiles "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
#
#target_sources( GraphicsHandler 
#    PRIVATE 
#        ${SourceFiles}
#    PUBLIC
#        ${HeadersFiles}
#)
find_package(directxtk CONFIG REQUIRED)

add_library(Graphics
    Graphics3D.cpp
    pch.cpp
    DeviceResourcesPC.cpp
    StepTimer.h
    RenderTexture.cpp
)

target_include_directories( Graphics
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
        imgui
)

target_link_libraries( Graphics PUBLIC
    d3dcompiler.lib
    lwlog::lwlog

    d3d11.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib
    Microsoft::DirectXTK
)