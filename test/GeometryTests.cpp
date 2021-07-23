#include "mycad/Geometry.h"
#include "Arbitrary.h"

#include <catch2/catch.hpp>
#include "rapidcheck.h"
#include "rapidcheck/catch.h"

using namespace mycad::geom;

SCENARIO( "001: Line Geometry", "[geometry][line]" ) {
    rc::prop("A line cannot be constructed with only one point",
        [](Point const &p1) {
            MaybeLine eLine = makeLine(p1, p1);
            RC_ASSERT_FALSE(eLine.has_value());
        },
        /* verbose= */ true
    );

    rc::prop("A line is parametrized from u=0 to u=1",
        []() {
            Point p1 = *rc::gen::arbitrary<Point>();
            Point p2 = *rc::gen::distinctFrom(p1);

            MaybeLine eLine = makeLine(p1, p2);
            RC_ASSERT(eLine.has_value());

            Line const &line = eLine.value();
            RC_ASSERT(line.atU(0) == p1);
            RC_ASSERT(line.atU(1) == p2);
        }
    );

    rc::prop("Any point on the line intersects the line",
        [](Line const &line) {
            float u = *rc::gen::inRange(0, 1);
            Point const &p = line.atU(u);
            RC_ASSERT(line.intersects(p));
        },
        /* verbose= */ true
    );

    rc::prop("Any point off the line does not intersect the line",
        [](Line const &line) {
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
