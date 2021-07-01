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

if(BUILD_TESTING)
    if(MYCAD_FETCH_CATCH)
        Include(FetchContent)

        FetchContent_Declare(
          Catch2
          GIT_REPOSITORY https://github.com/catchorg/Catch2.git
          GIT_TAG        v2.13.4)

        FetchContent_MakeAvailable(Catch2)

        set(CMAKE_MODULE_PATH
            "${catch2_SOURCE_DIR}/contrib;${CMAKE_MODULE_PATH}"
        )
    else()
        find_package(Catch2 2.13.4 REQUIRED)
    endif()

    add_subdirectory(ext/rapidcheck)

    set(MYCAD_RAPIDCHECK_EXTRAS_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/ext/rapidcheck/extras"
        PARENT_SCOPE
    )
endif()
