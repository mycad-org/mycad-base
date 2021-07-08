#include "mycad/Topology.h"

using namespace mycad::topo;

bool Link::operator==(const Link& other) const
{
    return parentVertex == other.parentVertex &&
           parentEdge == other.parentEdge;
}
