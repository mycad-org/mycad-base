#include "mycad/Entity.h"

using namespace mycad;

auto Entity::addVertex(Point const p) -> topo::VertexID
{
    auto v = topo.addFreeVertex();
    vertices.emplace(v, p);

    return v;
}

auto Entity::getPoint(topo::VertexID const v) -> Point
{
    return vertices.at(v);
}
