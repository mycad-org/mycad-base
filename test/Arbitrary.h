#include "mycad/Geometry.h"

#include "rapidcheck.h"

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

    template<>
    struct Arbitrary<Line> {
        static Gen<Line> arbitrary() {
            return gen::exec([]() {
                Point p1 = *gen::arbitrary<Point>();
                Point p2 = *gen::distinctFrom(p1);
                return makeLine(p1, p2).value();
                }
            );
        }
    };
}
