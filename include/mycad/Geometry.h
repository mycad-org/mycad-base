#ifndef MYCAD_GEOMETRY_HEADER
#define MYCAD_GEOMETRY_HEADER

#include <iostream>
#include <optional>
#include <utility>

#include "mycad/Types.h"

namespace mycad
{
    struct Point
    {
        float x, y, z;

        auto operator<=>(Point const&) const = default;
    };

    /** @brief Parametrized _from_ @param p1 _to_ @param p2
     *
     *  The paramatetrization results in a line described by:
     *
     *  @f[
     *      x = f(u)
     *      y = g(u)
     *      z = h(u)
     *  @f]
     *
     *  such that when @f$i u=0 @f$,
     *  @f$ x = p1.x, y = p1.y, z = y1.z @f$, and similarly for
     *  @f$ u=1 @f$ and @param p2.
     *
     *  @returns A Line if the two points are not equivalent
     */
    auto makeLine(Point const &p1, Point const &p2) -> MaybeLine;

    class Line {
        public:
            friend auto  makeLine(Point const &p1, Point const &p2) -> MaybeLine;

            /** @brief return the point at the given @param u
             *
             *  @returns Line#p1 when `u = 0`, Line#p2 when `u = 1`, the
             *           appropriate extrapolated point on the line otherwise
             */
            auto atU(float u) const -> Point;

            auto intersects(Point const &p) const -> bool;

            bool operator<=>(Line const&) const = default;
        private:
            Line(Point const &p1, Point const &p2);

            Point p1, p2;
    };

    auto operator<<(std::ostream &stream, Point const &p) -> std::ostream &;
    auto operator<<(std::ostream &stream, Line const &line)-> std::ostream &;

} // namespace mycad


#endif // MYCAD_GEOMETRY_HEADER
