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

Please see [the examples directory][./examples] for samples that fully compile
and run. Here are a few highlights:

```cpp
int main()
{
int main()
{
    // The plan is for mycad-entity to merge the functionalities of
    // mycad-geometry and mycad-topology. Something like this
    std::map<mycad::topo::VertexID, mycad::geom::Point> vertices;
    std::map<mycad::topo::EdgeID, mycad::geom::Line> edges;
    mycad::topo::Topology topo;

    // now the user can draw points, connect them into lines, and then we can
    // tell them later which are joined together.
    mycad::geom::Point p1(10, 10, 0); // similarly p2, p3, p4...

    // NOTE: we get the value directly from the `std::optional` because we are
    // very confident that these calls will succeed. Generally, this should be
    // checked programatically.
    auto l1 = mycad::geom::makeLine(p1, p2).value(); // l2, l3

    auto v1 = topo.addFreeVertex(); // v2, v3, v4

    auto e1 = topo.makeEdge(v1, v2); // e2, e3

    auto chain = topo.joinEdges(e1, e2);
    topo.joinEdges(e2, e3);

    // Here's a visual of what we've created
    //
    //   v4/p4         v1/p1
    // (0, 10, 0)   (10, 10, 0)
    //    ○              ○
    //    │              │
    //    │              │
    //    │e3/l3         │ e1/l2
    //    │              │
    //    │    e2/l2     │
    //    ○ ──────────── ○
    // (0,0,0)       (10, 0, 0)
    //  v3/p3          v2/p2

    auto edges = topo.getChainEdges(chain); // retrieves e1, e2, and e3
    auto [left, right] = topo.getEdgeVertices(e1); // retrieves v1 and v2
    auto left = topo.oppositeVertex(right, e1); // retrieves v1
}

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

Roadmap
=======

| Version | Target Date | Main Feature                                                                          |
| ------- | ----------- | ------------------------------------------------------------------------------------- |
|  v0.1   |  Aug. 2021  | entity can represent a chain of straight line segments in 3D space of arbitrary length|
|  v0.2   |  Oct. 2021  | `mycad-vis`, a minimal opengl viewer for mycad objects                                |
|  v0.3   |  Dec. 2021  | entity can represent a cube                                                           |
|  v0.4   |  Feb. 2022  | `arc` added to geometry - maybe `circle`                                              |
|  v0.5   |  Apr. 2022  | `splitEdge` added to topology                                                         |
|  v0.6   |  Jul. 2022  | `boolean union` function added to entity                                              |
|  v0.7   |  Sep. 2022  | `boolean subtract` function added to entity                                           |
|  v0.8   |  Nov. 2022  | `fillet` function added to entity                                                     |
|  v0.9   |  Dec. 2022  | [Topological naming problem][2] from FreeCAD is shown as solved                       |

[2]: https://wiki.freecadweb.org/Topological_naming_problem
