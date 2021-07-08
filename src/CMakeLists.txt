add_library(mycad_geometry Geometry.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(mycad-topology SHARED Topology.cpp Link.cpp Vertex.cpp detail/Topology.cpp)
