#include "mycad/Topology.h"

using namespace mycad::topo;

int Topology::addFreeVertex() {
    int out = lastVertexID++;
    vertexIDs.insert(out);
    return out;
}

const std::set<int> Topology::getEdgeIDs() {
    return edgeIDs;
}

tl::expected<int, std::string> Topology::makeEdge(int v1, int v2) {
    if (not (vertexIDs.contains(v1) && vertexIDs.contains(v2))) {
        return tl::unexpected(
            std::string("One or both of v1 = ") + std::to_string(v1) +
                        " and v2 = " + std::to_string(v2) +
                        " do not exist in the topology");
    }

    return lastEdgeID++;
}
