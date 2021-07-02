#include "mycad/Topology.h"

using namespace mycad::topo;

int Topology::addFreeVertex() {
    return lastVertexID++;
}
