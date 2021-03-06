#include "mycad/Topology.h"

#include <iostream>

int main()
{
    // A topology is used to keep track of relationships
    mycad::Topology topo;

    // A 'free' vertex is not connected to anything. Notice that Topology only
    // return an "ID" to the Vertex, not the Vertex itself.
    mycad::VertexID v1 = topo.addFreeVertex();
    mycad::VertexID v2 = topo.addFreeVertex();

    // Two free vertices can be connected by an Edge. Every Edge has exactly two
    // vertices associated with it
    mycad::MaybeEdgeID edge = topo.makeEdge(v1, v2);

    // Now, we can query the relationships
    auto [left, right] = topo.getEdgeVertices(edge).value();

    std::cout << "The edge with ID = " << edge << " is adjacent to:" << '\n'
              << "    Vertex ID = " << left << '\n'
              << "    Vertex ID = " << right << '\n';
    // output:
    // The edge with ID = 0 is adjacent to:
    //     Vertex ID = 0
    //     Vertex ID = 1

    // A series of Edges can be connected into a Chain
    auto v3 = topo.addFreeVertex();
    auto v4 = topo.addFreeVertex();
    auto v5 = topo.addFreeVertex();
    auto e2 = topo.makeEdge(v2, v3);
    auto e3 = topo.makeEdge(v3, v4);
    auto e4 = topo.makeEdge(v4, v5);

    // The first two Edges joined will result in the longest "chain"
    auto chain = topo.joinEdges(edge, e2);
    topo.joinEdges(e2, e3);
    auto shortChain = topo.joinEdges(e3, e4);

    // We can retrieve the Edges that have been chained together
    auto edges = topo.getChainEdges(*chain);
    auto shortEdges = topo.getChainEdges(*shortChain);

    // shortEdges will only contain a sub-set of edges - specifically, it will
    // only contain e3 and e4
    std::cout << "edges.has_value() = " << std::boolalpha << edges.has_value() << '\n';
    std::cout << "edges == shortEdges → " << std::boolalpha << (*edges == *shortEdges) << '\n';
}
