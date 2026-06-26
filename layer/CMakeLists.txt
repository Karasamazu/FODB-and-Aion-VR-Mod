cmake_minimum_required(VERSION 3.10)

project(aion_vr_layer LANGUAGES CXX)

find_package(Vulkan REQUIRED)

add_library(aion_vr_layer SHARED
    layer.cpp
    present_hook.cpp
    xr_bridge.cpp
    camera_hook.cpp
    util/log.cpp
    util/pattern_scan.cpp

)

target_link_libraries(aion_vr_layer PRIVATE Vulkan::Vulkan)

target_compile_definitions(aion_vr_layer PRIVATE XR_USE_GRAPHICS_API_VULKAN)

target_include_directories(aion_vr_layer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}        # layer/
    ${CMAKE_CURRENT_SOURCE_DIR}/util   # layer/util/
    ${CMAKE_CURRENT_SOURCE_DIR}/../    # Projekt-Root
    ${CMAKE_CURRENT_SOURCE_DIR}/../layer  # explizit layer/
)



set_target_properties(aion_vr_layer PROPERTIES
    OUTPUT_NAME "aion_vr_layer"
    PREFIX ""
)
