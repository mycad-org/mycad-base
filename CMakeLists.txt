cmake_minimum_required(VERSION 3.7)
project(mycad)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wextra -Wall -pedantic")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(MYCAD_EXT_TL_INCLUDE_DIR
    "${CMAKE_PROJECT_SOURCE_DIR}/ext/TartanLlama/include"
    CACHE
    PATH "Path to the TartanLlama c++ headers, i.e. tl::expected"
)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(ext/rapidcheck)

    set(MYCAD_RAPIDCHECK_EXTRAS_DIR
        "${CMAKE_PROJECT_SOURCE_DIR}/ext/rapidcheck/extras"
    )

    add_subdirectory(test)
endif()
