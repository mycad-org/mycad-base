#include "mycad/Geometry.h"

#include <cmath> // std::lerp (since c++20)
#include <iostream>

using namespace mycad::geom;

auto Line::makeLine(Point const &p1, Point const &p2) -> EitherLine
{
    if (p1 == p2)
    {
        return tl::make_unexpected(
            "A line cannot be constructed with two equivalent Points");
    }
    else
    {
        return Line(p1, p2);
    }
}

Line::Line(Point const &p1, Point const &p2)
    : p1(p1), p2(p2){}

Point Line::atU(float u) const
{
    if (u == 0)
    {
        return p1;
    }
    else if (u == 1)
    {
        return p2;
    }

    return Point(std::lerp(p1.x, p2.x, u),
                 std::lerp(p1.y, p2.y, u),
                 std::lerp(p1.z, p2.z, u)
            );
}

auto Line::intersects(Point const &p) const -> bool
{
    if (p == p1 || p == p2)
    {
        return true;
    }

    // The parametric equations of a 3d line are:
    //
    // x = x₁ + (x₂ - x₁)u
    // y = y₁ + (y₂ - y₁)u
    // z = z₁ + (z₂ - z₁)u
    //
    // We can use this to solve for `u` given any one of the components of the
    // point we were given
    float u = (p.x - p1.x) / (p2.x - p1.x);

    // Now we should be able to get the point back if it intersects with our
    // line
    return p == this->atU(u);
}

auto mycad::geom::operator<<(std::ostream &stream, Point const &p) -> std::ostream &
{
    stream << "(" << p.x << ", " << p.y << ", " << p.z << ")";
    return stream;
}

auto mycad::geom::operator<<(std::ostream &stream, Line const &line) -> std::ostream &
{
    stream << "Line: " << line.atU(0) << " → " << line.atU(1);
    return stream;
}
