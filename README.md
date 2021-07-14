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
    // ================   Geometry ============================

    mycad::geom::Point p1(10, 20, 30);
    mycad::geom::Point p2(40, 50, 60);

    // Return type is tl::unexpected - provides error handling
    auto eitherLine1 = mycad::geom::makeLine(p1, p2);

    if (eitherLine1)
    {
        // this is safe now
        mycad::geom::Line line = eitherLine1.value();
        //... do something with the line
    }
    else
    {
        // handle the error - maybe ask the user to try again
    }

    // ================   Topology  ============================

    // A topology is used to keep track of relationships
    mycad::topo::Topology topo;

    // 'Free' vertices aren't connected to anything
    mycad::topo::VertexID v1 = topo.addFreeVertex();
    mycad::topo::VertexID v2 = topo.addFreeVertex();

    // These vertices are no longer 'Free'.
    // Notice we used unsafe_makeEdge - no error checking is done, be careful!
    mycad::topo::EdgeID edge = topo.unsafe_makeEdge(v1, v2);

    // Now, we can query the relationships
    auto eitherVertices = topo.getEdgeVertices(edge);
    if(eitherVertices)
    {
        // .value is safe now
        auto [left, right] = eitherVertices.value();

        std::cout << "The edge with ID = " << edge.index << " is adjacent to:" << '\n'
                  << "    Vertex ID = " << left.index << '\n'
                  << "    Vertex ID = " << right.index << '\n';
        /* output:
         * The edge with ID = 0 is adjacent to:
         *     Vertex ID = 0
         *     Vertex ID = 1
         */
    }

    // ================   Debugging  ============================
    std::cout << "P1: " << p1 << '\n'
              << "P2: " << p2 << '\n'
              << "L1: " << eitherLine.value() << '\n';
              << '\n' << "Topology:" << '\n\n'
              << topo << '\n';
    /* output
     *
     *  P1: (10, 20, 30)
     *  P2: (40, 50, 60)
     *  L1: Line: (10, 20, 30) → (40, 50, 60)
     *
     *  Topology:
     *
     *  lastVertexID = 2, lastEdgeID = 1
     *  vertexIDs:
     *      vid: 0
     *          link
     *              parentVertex = 0
     *              parentEdge   = 0
     *      vid: 1
     *          link
     *              parentVertex = 1
     *              parentEdge   = 0
     *  edges:
     *      eid: 0
     *          leftVertexID = 0
     *          rightVertexID = 1
     */
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
