#include "mycad/Entity.h"

using namespace mycad;

auto Entity::addVertex(Point const p) -> VertexID
{
    auto v = topo.addFreeVertex();
    vertices.emplace(v, p);

    return v;
}

auto Entity::addEdge(VertexID const v1, VertexID const v2) -> MaybeEdgeID
{
    auto maybeEdge = topo.makeEdge(v1, v2);

    // This also ensures that the topology contains v1 and v2, otherwise the
    // edge would not have been....er...."makeable"
    if(not maybeEdge.has_value())
    {
        return std::nullopt;
    }

    auto maybeLine = mycad::makeLine(vertices.at(v1), vertices.at(v2));

    if(not maybeLine.has_value())
    {
        return std::nullopt;
    }

    edges.emplace(*maybeEdge, *maybeLine);

    return maybeEdge;
}

auto Entity::getPoint(VertexID const v) const -> Point
{
    return vertices.at(v);
}

auto Entity::getLine(EdgeID const e) const -> MaybeLine
{
    if(topo.hasEdge(e))
    {
        return edges.at(e);
    }

    return std::nullopt;
}

auto Entity::getEdges() const -> Lines
{
    Lines out;
    for (auto const & [_, line] : edges)
    {
        out.push_back(line);
    }

    return out;
}
