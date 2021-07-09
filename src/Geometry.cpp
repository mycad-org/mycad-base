#include "mycad/Geometry.h"

#include <iostream>

using namespace mycad::geom;

Point::Point(float x, float y, float z)
    : x(x), y(y), z(z){}

bool Point::operator==(const Point& other) const {
    return x == other.x && y == other.y && z == other.z;
}

Line::Line(const Point& p1, const Point& p2)
    : p1(p1), p2(p2){}

Point Line::atU(float u) const {
    auto f = [u](float component1, float component2) -> float {
            return component1 + u * (component2 - component1);
        };

    return Point(f(p1.x, p2.x),
                 f(p1.y, p2.y),
                 f(p1.z, p2.z)
            );
}

std::ostream& mycad::geom::operator<<(
        std::ostream& stream,
        const mycad::geom::Point& p)
{
    stream << "(" << p.x << ", " << p.y << ", " << p.z << ")";
    return stream;
}
