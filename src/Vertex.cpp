#include "mycad/Topology.h"

#include <string>

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
    os << std::string("V") << std::to_string(index);
}
