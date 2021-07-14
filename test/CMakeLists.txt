if(MYCAD_FETCH_CATCH)
    Include(FetchContent)

    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v2.13.4)

    FetchContent_MakeAvailable(Catch2)

    set(CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib;${CMAKE_MODULE_PATH}")
else()
    find_package(Catch2 2.13.4 REQUIRED)
endif()

set(TEST_SRCS
    main.cpp
    GeometryTests.cpp
    TopologyTests.cpp
    )

set(TEST_LIBS
    Catch2::Catch2
    mycad-geometry
    mycad-topology
    rapidcheck
    )

add_executable(mycad_tests ${TEST_SRCS})
target_link_libraries(mycad_tests PUBLIC ${TEST_LIBS} INTERFACE rapidcheck_catch)
target_include_directories(
    mycad_tests PUBLIC ${rapidcheck_SOURCE_DIR}/extras/catch/include)

include(Catch REQUIRED) # provides catch_discover_tests
catch_discover_tests(mycad_tests)

add_executable(mycad_example example.cpp)
target_link_libraries(mycad_example mycad-geometry mycad-topology)
