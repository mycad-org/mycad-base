add_library(mycad-geometry SHARED Geometry.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library( mycad-topology SHARED Topology.cpp detail/Topology.cpp)
