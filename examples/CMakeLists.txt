add_executable(full_demo full_demo.cpp)
target_link_libraries(full_demo mycad-geometry mycad-topology)

add_executable(geometry_demo geometry.cpp)
target_link_libraries(geometry_demo mycad-geometry)

add_executable(topology_demo topology.cpp)
target_link_libraries(topology_demo mycad-topology)
