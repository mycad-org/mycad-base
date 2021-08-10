add_library(mycad-geometry SHARED Geometry.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library( mycad-topology SHARED detail/Topology.cpp Topology.cpp)

add_library(mycad-entity SHARED Entity.cpp)
target_link_libraries(mycad-entity mycad-geometry mycad-topology)

add_executable(mycad-vis main.cpp GLFW_Application.cpp GL_Renderer.cpp)
target_link_libraries(mycad-vis glfw GLEW GL)

if(MYCAD_VULKAN_VIEWER)
    find_package(Vulkan REQUIRED)
    add_executable(mycad-vk vulkan_main.cpp vulkan_helpers.cpp)
    target_link_libraries(mycad-vk glfw ${Vulkan_LIBRARIES} ${CMAKE_DL_LIBS})
endif()
