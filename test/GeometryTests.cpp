#include <catch2/catch.hpp>

#include "mycad/Geometry.h"

SCENARIO( "001: Line Geometry", "[geometry][line]" ) {
    GIVEN("Two points") {
        mycad::geom::Point p1(10, 20, 30),
                           p2(40, 50, 60);

        WHEN("A Line is created") {
            mycad::geom::Line line(p1, p2);
            THEN("It is parametrized from u=0 to u=1") {
                REQUIRE(line.atU(0) == p1);
                REQUIRE(line.atU(1) == p2);
            }
        }
    }
}
