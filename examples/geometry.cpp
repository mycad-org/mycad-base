#include "mycad/Geometry.h"

int main()
{
    // You might get an error if you don't compile with `-std=c++20`.
    // This is because we're using the parenthesized aggregate initialization
    // feature added in c++20
    mycad::geom::Point p1(10, 20, 30);
    mycad::geom::Point p2(40, 50, 60);

    // Streaming operators for handy 'debugging'
    std::cout << "P1 = " << p1 << '\n';
    std::cout << "P2 = " << p2 << '\n';
    // output: P1 = (10, 20, 30)
    //         P2 = (40, 50, 60)

    // Return type is std::optional
    auto maybeLine1 = mycad::geom::makeLine(p1, p2);
    auto maybeLine2 = mycad::geom::makeLine(p1, p1);

    if (maybeLine1)
    {
        // this is safe now
        mycad::geom::Line line = maybeLine1.value();

        // Again, handy streaming operator
        std::cout << line << '\n';
        // output: Line: (10, 20, 30) → (40, 50, 60)
    }

    if(not maybeLine2)
    {
        // oh no, something happened!
        // maybe ask the user to try again? in a loop?
    }
}
