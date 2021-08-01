add_library(mycad-geometry SHARED Geometry.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library( mycad-topology SHARED detail/Topology.cpp Topology.cpp)

add_library(mycad-entity SHARED Entity.cpp)
target_link_libraries(mycad-entity mycad-geometry mycad-topology)

add_executable(mycad-vis main.cpp Viewer.cpp)
target_link_libraries(mycad-vis glfw GL GLEW)
