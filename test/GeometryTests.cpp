#include "mycad/Geometry.h"
#include <tuple>

#include <catch2/catch.hpp>
#include "rapidcheck.h"
#include "rapidcheck/catch.h"

using namespace mycad::geom;

namespace rc{
    template<>
    struct Arbitrary<Point> {
        static Gen<Point> arbitrary() {
            return gen::build<Point>(
                gen::set(&Point::x),
                gen::set(&Point::y),
                gen::set(&Point::z)
            );
        }
    };
}

SCENARIO( "001: Line Geometry", "[geometry][line]" ) {
    GIVEN("Two points") {
        WHEN("A Line is created") {
            THEN("It is parametrized from u=0 to u=1") {
            rc::prop("",
                [](const Point& p1, const Point& p2) {
                    Line line(p1, p2);
                    RC_ASSERT(line.atU(0) == p1);
                    RC_ASSERT(line.atU(1) == p2);
                }
            );
            }
        }
    }
}
