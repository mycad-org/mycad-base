#include "mycad/Geometry.h"

#include "rapidcheck.h"

namespace rc{
    template<>
    struct Arbitrary<mycad::Point> {
        static Gen<mycad::Point> arbitrary() {
            return gen::build<mycad::Point>(
                gen::set(&mycad::Point::x),
                gen::set(&mycad::Point::y),
                gen::set(&mycad::Point::z)
            );
        }
    };

    template<>
    struct Arbitrary<mycad::Line> {
        static Gen<mycad::Line> arbitrary() {
            return gen::exec([]() {
                auto p1 = *gen::arbitrary<mycad::Point>();
                auto p2 = *gen::distinctFrom(p1);
                return makeLine(p1, p2).value();
                }
            );
        }
    };
}
