cmake_minimum_required(VERSION 3.7)
project(mycad)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wextra -Wall -pedantic")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(MYCAD_BUILD_EXAMPLES FALSE CACHE BOOL "Whether to build the examples programs")
set(MYCAD_VULKAN_VIEWER FALSE CACHE BOOL "Whether to build the mycad-vk viewer")

include_directories(include)
add_subdirectory(src)

if(MYCAD_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

include(CTest)

if(BUILD_TESTING)
    enable_testing()

    add_subdirectory(ext/rapidcheck)

    add_subdirectory(test)
endif()
