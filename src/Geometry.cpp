#include "mycad/Geometry.h"

#include <iostream>

using namespace mycad::geom;

Point::Point(float x, float y, float z)
    : x(x), y(y), z(z){}

bool Point::operator==(const Point& other) const {
    return x == other.x && y == other.y && z == other.z;
}

tl::expected<Line, std::string> Line::makeLine(const Point& p1, const Point& p2)
{
    if (p1 == p2)
    {
        return tl::unexpected(std::string("A line cannot be constructed with two equivalent Points"));
    }
    else
    {
        return Line(p1, p2);
    }
}

Line::Line(const Point& p1, const Point& p2)
    : p1(p1), p2(p2){}

Point Line::atU(float u) const {
    if (u == 0)
    {
        return p1;
    }
    else if (u == 1)
    {
        return p2;
    }

    auto f = [u](float component1, float component2) -> float {
            return component1 + u * (component2 - component1);
        };

    return Point(f(p1.x, p2.x),
                 f(p1.y, p2.y),
                 f(p1.z, p2.z)
            );
}

bool Line::intersects(const Point& ) const {
    return false;
}

std::ostream& mycad::geom::operator<<(
        std::ostream& stream,
        const mycad::geom::Point& p)
{
    stream << "(" << p.x << ", " << p.y << ", " << p.z << ")";
    return stream;
}

std::ostream& mycad::geom::operator<<(
        std::ostream& stream,
        const mycad::geom::Line& line)
{
    stream << "Line: " << line.atU(0) << " → " << line.atU(1);
    return stream;
}
