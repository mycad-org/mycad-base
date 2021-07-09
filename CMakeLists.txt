cmake_minimum_required(VERSION 3.7)
project(mycad)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wextra -Wall -pedantic")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

include_directories(include ext/TartanLlama/include)
add_subdirectory(src)
include(CTest)

if(BUILD_TESTING)
    add_subdirectory(ext/rapidcheck)

    add_subdirectory(test)
endif()
