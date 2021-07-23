[![build_and_tests](https://github.com/mycad-org/mycad-base/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/mycad-org/mycad-base/actions/workflows/unit_tests.yml)

Overview
========

This library provides the fundamental building blocks needed to implement a
Computer Aided Design (CAD) package. It is broken into three libraries:

- mycad-geometry
- mycad-topology
- [future] mycad-entity

The `mycad-geometry` and `mycad-topology` libaries are completely independent of
each other. Their only dependencies are the traditional c/c++ runtimes and
standard libraries. You are welcome and encouraged to use either (or both) in
your own projects.

Example
=======

Please see [this example][./test/examplec.cpp] for a fully compilable and
runnable example. Below you'll get some highlights:

```cpp
int main()
{
    // All functionality is provided in the `mycad` namespace

    mycad::geom::Point p1(10, 20, 30);
    mycad::geom::Point p2(40, 50, 60);

    // Some function return a `std::optional`
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

    //.... these geometric facilities will grow and scale with the needs of mycad

    /*======================================================================*/

    // A topology is used to keep track of relationships
    mycad::topo::Topology topo;

    // A 'free' vertex is not connected to anything. Notice that Topology only
    // return an "ID" to the Vertex, not the Vertex itself.
    mycad::topo::VertexID v1 = topo.addFreeVertex();
    mycad::topo::VertexID v2 = topo.addFreeVertex();

    // Two free vertices can be connected by an Edge. Every Edge has exactly two
    // vertices associated with it
    mycad::topo::EdgeID edge = topo.makeEdge(v1, v2);

    // Now, we can query the relationships
    auto [left, right] = topo.getEdgeVertices(edge);

    std::cout << "The edge with ID = " << edge.index << " is adjacent to:" << '\n'
              << "    Vertex ID = " << left.index << '\n'
              << "    Vertex ID = " << right.index << '\n';

    // output:
    // The edge with ID = 0 is adjacent to:
    //     Vertex ID = 0
    //     Vertex ID = 1
}

```

Building
========

This project can be built using cmake. It uses some c++20 features, so please
check your compiler's compatibility with c++20 if you have problems.

If the c++20 stuff is an issue for you, let me know and I can remove it.

```sh
mkdir build
cd build
cmake ..
make
```

If you want to run the unit tests, you'll need to install the [Catch2][1]
library. If you don't want to (or can't) install it, then you can have cmake
fetch a local copy with:

```sh
cmake -DMYCAD_FETCH_CATCH=ON ..
```

[1]: https://github.com/catchorg/Catch2
