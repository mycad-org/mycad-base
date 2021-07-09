#include "mycad/Geometry.h"

#include <iostream>

using namespace mycad::geom;

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

bool Line::intersects(const Point& p) const {
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
