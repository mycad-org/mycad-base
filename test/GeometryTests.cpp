#include "mycad/Geometry.h"
#include "tl/expected.hpp"

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
    rc::prop("A line cannot be constructed with only one point",
        [](const Point& p1) {
            tl::expected<Line, std::string>  eLine = Line::makeLine(p1, p1);
            RC_ASSERT_FALSE(eLine.has_value());
        }
    );
    rc::prop("A line is parametrized from u=0 to u=1",
        []() {
            Point p1 = *rc::gen::arbitrary<Point>();
            Point p2 = *rc::gen::distinctFrom(p1);

            auto eLine = Line::makeLine(p1, p2);
            RC_ASSERT(eLine.has_value());
            const Line& line = eLine.value();
            RC_ASSERT(line.atU(0) == p1);
            RC_ASSERT(line.atU(1) == p2);
        }
    );
}
