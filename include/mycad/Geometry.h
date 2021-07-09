#ifndef MYCAD_GEOMETRY_HEADER
#define MYCAD_GEOMETRY_HEADER

namespace mycad {
    namespace geom {
        struct Point {
            float x, y, z;

            Point(float x, float y, float z);

            bool operator==(const Point& other) const;
        };

        class Line {
            public:
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
                 */
                Line(const Point& p1, const Point& p2);

                /** @brief return the point at the given @param u
                 *
                 *  @returns Line#p1 when `u = 0`, Line#p2 when `u = 1`, the
                 *           appropriate extrapolated point on the line
                 *           otherwise
                 */
                Point atU(float u) const;

            private:
                Point p1, p2;
        };
    }
}
#endif // MYCAD_GEOMETRY_HEADER
