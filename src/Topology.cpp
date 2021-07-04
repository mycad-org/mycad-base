#include "mycad/Topology.h"
#include "detail/Topology.h"

#include <iostream>

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

/**
 * The two Vertices **can** be the same, in which case the Edge would be
 * considered a "loop" edge.
 *
 * Only a single Edge can exist between two Vertices in a given direction, so
 * that if this method is called twice with the same vertices an error is
 * returned.
 *
 * However, if this method is called a second time with the parameters reversed,
 * this is valid and results in an Edge going "in the other direction".
 *
 * Visually:
 *
 * makeEdge(v1, v2): V₁ → V₂
 * makeEdge(v1, v2): error
 * makeEdge(v2, v1): V₁ ↔ V₂
 *
 * @returns error string if either of the vertices doesn't exist
 * @returns error string if an Edge already exists **from** @v1@ **to** @v2@
 */
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
    int eid = lastEdgeID++;
    edges.emplace(eid,
                  std::make_unique<detail::Edge>(v1, v2));
    return eid;
}

tl::expected<std::unordered_set<int>, std::string>
Topology::edgesAdjacentToVertex(int v) const
{
    return detail::hasVertex(v, vertexIDs)
           .map([v, this]
           {
               std::unordered_set<int> out;
               for (const auto& [edgeID, edge] : edges)
               {
                   if (edge->leftVertexID == v || edge->rightVertexID == v)
                   {
                       out.insert(edgeID);
                   }
               }
               return out;
           });
}

void Topology::streamTo(std::ostream& os) const
{
    os << "lastVertexID = " << lastVertexID << ", "
       << "lastEdgeID = " << lastEdgeID << std::endl;

    os << "vertexIDs:" << std::endl;;

    for (const auto& id : vertexIDs)
    {
        os << "    vid: " << id << std::endl;
    }

    os << "edges:" << std::endl;
    for (const auto& [key, edge] : edges)
    {
        os << "    eid: " << key << ", "
           << "leftVertexID = " << edge->leftVertexID << ", "
           << "rightVertexID = " << edge->rightVertexID << std::endl;
    }
}

std::ostream& mycad::topo::operator<<(std::ostream& os, const Topology& topo)
{
    topo.streamTo(os);
    return os;
}
