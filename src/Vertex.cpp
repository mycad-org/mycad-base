#include "mycad/Topology.h"

using namespace mycad::topo;

Vertex::Vertex(int i)
    : index(i)
{}

bool Vertex::operator<(const Vertex& other) const
{
    return index < other.index;
}

bool Vertex::operator== (const Vertex& other) const
{
    return index == other.index &&
           links == other.links;
}

int Vertex::getIndex() const
{
    return index;
}


void Vertex::streamTo(std::ostream& os) const
{
    os << "V" << index;
}
