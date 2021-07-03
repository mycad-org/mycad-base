#include "mycad/Topology.h"
#include "detail/Topology.h"
#include "detail/Topology.cpp"

using namespace mycad::topo;

Topology::Topology() = default;

Topology::Topology(const Topology& other)
    : lastVertexID(other.lastVertexID), lastEdgeID(other.lastEdgeID), vertexIDs(other.vertexIDs)
{
    for(const auto& [id, edge] : other.edges)
    {
        edges.emplace(id, std::make_unique<detail::Edge>(edge->leftVertexID,
                                                         edge->rightVertexID));
    }
}

Topology::~Topology() = default;

Topology& Topology::operator=(const Topology& other)
{
    if (this == &other)
    {
        return *this;
    }

    this->lastVertexID = other.lastVertexID;
    this->lastEdgeID = other.lastEdgeID;
    this->vertexIDs = other.vertexIDs;

    edges.clear();
    for(const auto& [id, edge] : other.edges)
    {
        edges.emplace(id, std::make_unique<detail::Edge>(edge->leftVertexID,
                                                         edge->rightVertexID));
    }
    return *this;
}

int Topology::addFreeVertex()
{
    int out = lastVertexID++;
    vertexIDs.insert(out);
    return out;
}

tl::expected<int, std::string> Topology::makeEdge(int v1, int v2)
{
    if (not (vertexIDs.contains(v1) && vertexIDs.contains(v2)))
    {
        return tl::unexpected(
            std::string("One or both of v1 = ") + std::to_string(v1) +
                        " and v2 = " + std::to_string(v2) +
                        " do not exist in the topology");
    }

    for (const auto& [uid, edge] : edges)
    {
        if (edge->leftVertexID == v1 && edge->rightVertexID == v2)
        {
            return tl::unexpected(
                std::string("Only one Edge can exist between two Vertices in"
                            " a given direction, i.e. v₁ → v₂. Maybe try "
                            "flipping the arguments to make v₂ → v₁"));
        }
    }

    detail::Edge edge(v1, v2);
    edges.emplace(lastEdgeID++,
                  std::make_unique<detail::Edge>(v1, v2));
    return lastEdgeID;
}
