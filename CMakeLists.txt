cmake_minimum_required(VERSION 3.22)
include(VulkanApp.cmake)
project(VulkanApp VERSION 1.0)

find_package(Vulkan REQUIRED) # Vulkan_INCLUDE_DIRS, Vulkan_LIBRARIES, Vulkan_VERSION
message(STATUS "Vulkan:")
message(NOTICE "   version = ${Vulkan_VERSION}")
message(NOTICE "   include dir = ${Vulkan_INCLUDE_DIRS}")
message(NOTICE "   libraries = ${Vulkan_LIBRARIES}")

git_submodule_update("${PROJECT_SOURCE_DIR}/vendor/assimp/CMakeLists.txt")

set(BUILD_SHARED_LIBS ON)
add_subdirectory(vendor/glfw)

set(CMAKE_POLICY_DEFAULT_CMP0175 OLD) #disables warning
add_subdirectory(vendor/assimp)
set(CMAKE_POLICY_DEFAULT_CMP0175 NEW)

set(ASSIMP_DLL_FILE "")
get_target_property(ASSIMP_DLL_FILE assimp OUTPUT_NAME)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(IMGUI
    vendor/imgui/imstb_truetype.h
    vendor/imgui/imconfig.h
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui.h
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_internal.h
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imstb_rectpack.h
    vendor/imgui/imstb_textedit.h
    vendor/imgui/backends/imgui_impl_glfw.h
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_vulkan.h
    vendor/imgui/backends/imgui_impl_vulkan.cpp
)
source_group(ImGUI FILES ${IMGUI})

set(SRC
    src/Axes.cpp
    src/Axes.h
    src/Camera.h
    src/CameraController.cpp
    src/CameraController.h
    src/Components.h
    src/Device.cpp
    src/Device.h
    src/FPS.h
    src/GameObject.h
    src/Grid.cpp
    src/Grid.h
    src/main.cpp
    src/Material.cpp
    src/Material.h
    src/Mesh.cpp
    src/Mesh.h
    src/Model.cpp
    src/Model.h
    src/Pipeline.cpp
    src/Pipeline.h
    src/Prism.cpp
    src/Prism.h
    src/RenderImage.cpp
    src/RenderImage.h
    src/Shader.cpp
    src/Shader.h
    src/Swapchain.cpp
    src/Swapchain.h
    src/Texture.cpp
    src/Texture.h
    src/Timer.h
    src/Transform.h
    src/ValidationLayers.cpp
    src/ValidationLayers.h
    src/VulkanApp.cpp
    src/VulkanApp.h
    src/Window.cpp
    src/Window.h
)
source_group(src   FILES ${SRC})

add_executable(VulkanApp)

set_target_properties(
    VulkanApp PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanApp)

target_include_directories(VulkanApp PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    vendor/glm
    vendor/stb
    vendor/eventpp/include
    vendor/spdlog/include
    vendor/imgui
)

target_sources(VulkanApp PRIVATE ${SRC} ${IMGUI})

target_link_libraries(VulkanApp PRIVATE
    ${Vulkan_LIBRARIES}
    glfw
    assimp
)

copy_dlls(VulkanApp)