#include "mycad/Topology.h"

#include <string>

using namespace mycad::topo;

Edge::Edge(int e, int l, int r)
    : index(e), leftVertexID(l), rightVertexID(r)
{}

int Edge::getIndex() const
{
    return index;
}

std::pair<int, int> Edge::getVertexIDs() const
{
    return {leftVertexID, rightVertexID};
}

void Edge::streamTo(std::ostream& os) const
{
    os << std::string("E") << std::to_string(index);
}
