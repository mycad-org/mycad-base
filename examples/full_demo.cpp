#include "mycad/Geometry.h"
#include "mycad/Topology.h"

#include <iostream>
#include <map>

int main()
{
    // The plan is for mycad-entity to merge the functionalities of
    // mycad-geometry and mycad-topology. Something like this
    std::map<mycad::VertexID, mycad::Point> vertices;
    std::map<mycad::EdgeID, mycad::Line> edges;
    mycad::Topology topo;

    // now the user can draw points, connect them inte lines, and then we can
    // tell them later which are joined together.
    mycad::Point p1(10, 10, 0);
    mycad::Point p2(10,  0, 0);
    mycad::Point p3( 0,  0, 0);
    mycad::Point p4( 0, 10, 0);

    // NOTE: we get the value directly from the `std::optional` because we are
    // very confident that these calls will succeed. Generally, this should be
    // checked programatically.
    auto l1 = mycad::makeLine(p1, p2).value();
    auto l2 = mycad::makeLine(p2, p3).value();
    auto l3 = mycad::makeLine(p3, p4).value();

    auto v1 = topo.addFreeVertex();
    auto v2 = topo.addFreeVertex();
    auto v3 = topo.addFreeVertex();
    auto v4 = topo.addFreeVertex();

    auto e1 = topo.makeEdge(v1, v2);
    auto e2 = topo.makeEdge(v2, v3);
    auto e3 = topo.makeEdge(v3, v4);

    [[maybe_unused]] auto chain = topo.joinEdges(e1, e2);
    topo.joinEdges(e2, e3);

    vertices[v1] = p1;
    vertices[v2] = p2;
    vertices[v3] = p3;
    vertices[v4] = p4;

    edges.emplace(e1, l1);
    edges.emplace(e2, l2);
    edges.emplace(e3, l3);


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

    // Now the user can very easily query information. Let's say they want to
    // close the box. They might do so using the following steps:
    //
    // 1. Utilize some GUI component that says "Add edge between points"
    // 2. Click on v1
    // 3. Click on v2

    // The information that we've gathered so far gives us enough to go on to
    // complete the request:
    //
    // 1. lookup p1 and p2 in the vertices map
    // 2, create a `mycad::Line` between the two points
    // 3. update the topology accordingly
}

