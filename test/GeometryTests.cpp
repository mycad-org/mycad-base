#include "mycad/Geometry.h"
#include "tl/expected.hpp"

#include <catch2/catch.hpp>
#include "rapidcheck.h"
#include "rapidcheck/catch.h"

using namespace mycad::geom;
using EitherLineOrString = tl::expected<Line, std::string>;

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

    template<>
    struct Arbitrary<Line> {
        static Gen<Line> arbitrary() {
            return gen::exec([]() {
                Point p1 = *gen::arbitrary<Point>();
                Point p2 = *gen::distinctFrom(p1);
                return Line::makeLine(p1, p2).value();
                }
            );
        }
    };
}

SCENARIO( "001: Line Geometry", "[geometry][line]" ) {
    rc::prop("A line cannot be constructed with only one point",
        [](const Point& p1) {
            EitherLineOrString eLine = Line::makeLine(p1, p1);
            RC_ASSERT_FALSE(eLine.has_value());
        },
        /* verbose= */ true
    );

    rc::prop("A line is parametrized from u=0 to u=1",
        []() {
            Point p1 = *rc::gen::arbitrary<Point>();
            Point p2 = *rc::gen::distinctFrom(p1);

            EitherLineOrString eLine = Line::makeLine(p1, p2);
            RC_ASSERT(eLine.has_value());

            const Line& line = eLine.value();
            RC_ASSERT(line.atU(0) == p1);
            RC_ASSERT(line.atU(1) == p2);
        }
    );

    rc::prop("Any point on the line intersects the line",
        [](const Line& line) {
            float u = *rc::gen::inRange(0, 1);
            const Point& p = line.atU(u);
            RC_ASSERT(line.intersects(p));
        },
        /* verbose= */ true
    );

    rc::prop("Any point off the line does not intersect the line",
        [](const Line& line) {
            float u = *rc::gen::inRange(0, 1);
            Point p = line.atU(u);

            // This is probably more complex than it needs to be, but what we're
            // going to do is offset one or more of the (x,y,z) components of
            // the point `p` by a random non-zero amount. This way, we know that
            // the point is not on the line, but we've randomized how far off
            // the line it is
            int n = *rc::gen::inRange<int>(1, 3);
            for (int i = 0; i < n; i++)
            {
                float delta = *rc::gen::nonZero<float>();
                if (i == 0)
                {
                    p.x += delta;
                }
                else if (i == 1)
                {
                    p.y += delta;
                }
                else if (i == 2)
                {
                    p.z += delta;
                }
            }
            RC_ASSERT_FALSE(line.intersects(p));
        },
        /* verbose= */ true
    );
}
