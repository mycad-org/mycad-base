#include "mycad/Geometry.h"
#include "mycad/Topology.h"

#include <iostream>
#include <map>

int main()
{
    // All functionality is provided in the `mycad` namespace

    mycad::geom::Point p1(10, 20, 30);
    mycad::geom::Point p2(40, 50, 60);

    // Streaming operators for handy 'debugging'
    std::cout << "P1 = " << p1 << '\n';
    // output: P1 = (10, 20, 30)

    std::cout << "P2 = " << p2 << '\n';
    // output: P2 = (40, 50, 60)

    // Return type is tl::unexpected - provides error handling
    auto eitherLine1 = mycad::geom::makeLine(p1, p2);
    auto eitherLine2 = mycad::geom::makeLine(p1, p1);

    if (eitherLine1)
    {
        // this is safe now
        mycad::geom::Line line = eitherLine1.value();

        // Again, handy streaming operator
        std::cout << line << '\n';
        // output: Line: (10, 20, 30) → (40, 50, 60)
    }

    if(not eitherLine2)
    {
        // oh no, something happened!

        std::cout << eitherLine2.error() << '\n';
        // output: A line cannot be constructed with two equivalent points


        // maybe ask the user to try again? in a loop?
    }

    //.... these geometric facilities will grow and scale with the needs of
    //     mycad

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
    auto eitherVertices = topo.getEdgeVertices(edge);
    if(eitherVertices)
    {
        // .value is safe now
        auto [left, right] = eitherVertices.value();

        std::cout << "The edge with ID = " << edge.index << " is adjacent to:" << '\n'
                  << "    Vertex ID = " << left.index << '\n'
                  << "    Vertex ID = " << right.index << '\n';
        // output:
        // The edge with ID = 0 is adjacent to:
        //     Vertex ID = 0
        //     Vertex ID = 1
    }

    /*======================================================================*/

    // The plan is for mycad-entity to merge the functionalities of these two
    // libraries in order to provide the building blocks for a boundary
    // representation CAD package. Something like
    std::map<mycad::topo::VertexID, mycad::geom::Point> vertices;
    std::map<mycad::topo::EdgeID, mycad::geom::Line> curves;

    // now the user can draw points, connect them inte lines, and then we can
    // tell them later which are joined together.
    //
    std::cout << "P1: " << p1 << '\n'
              << "P2: " << p2 << '\n'
              << "L1: " << eitherLine1.value() << '\n'
              << topo << '\n';
}

