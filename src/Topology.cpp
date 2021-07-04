#include "mycad/Topology.h"
#include "detail/Topology.h"

#include <iostream>

using namespace mycad::topo;

Topology::Topology() = default;

Topology::Topology(const Topology& other)
    : lastVertexID(other.lastVertexID), lastEdgeID(other.lastEdgeID), vertices(other.vertices)
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
    this->vertices = other.vertices;

    edges.clear();
    for(const auto& [id, edge] : other.edges)
    {
        edges.emplace(id, std::make_unique<detail::Edge>(edge->leftVertexID,
                                                         edge->rightVertexID));
    }
    return *this;
}

/**
 *
 *  Each Topology keeps track of a list of UIDs for each topological entity. IDs
 *  are not reused. Therefore:
 *
 *  ```cpp
 *  Topology topo1;
 *  int v1 = topo1.addFreeVertex();
 *  int v2 = topo1.addFreeVertex();
 *
 *  Topology topo2(topo1);
 *
 *  int eid = topo1.makeEdge(v1, v2);
 *  topo1.deleteEdge(eid);
 *
 *  topo1 == topo2;       // false (1)
 *  topo1.similar(topo2); // true (2)
 *  topo2.similar(topo1); // true (2)
 *  ```
 *
 *  The reason these two topologies are not equivalent is because topo1 has more
 *  "history". It knows about the Edge that was created and then deleted. Since
 *  UID's are not reused, the next time an Edge is made, it will get the "next"
 *  availabl ID.
 *
 *  The specifics of how the ID's are created and managed is an implementation
 *  detail, but it suffices to know that this mechanism is what makes `topo1 !=
 *  topo2`
 */
bool Topology::similar(const Topology& other) const
{
    return vertices == other.vertices && edges == other.edges;
}

int Topology::addFreeVertex()
{
    int out = lastVertexID++;
    vertices.insert(out);
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
    if (not (vertices.contains(v1) && vertices.contains(v2)))
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

bool Topology::deleteEdge(int edge)
{
    auto search = edges.find(edge);
    if (search == edges.end())
    {
        return false;
    }

    edges.erase(search);
    return true;
}

tl::expected<std::unordered_set<int>, std::string>
Topology::edgesAdjacentToVertex(int v) const
{
    return detail::hasVertex(v, vertices)
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

tl::expected<std::pair<int, int>, std::string>
Topology::getEdgeVertices(int edge) const
{
    return detail::hasEdge(edge, edges)
           .map([edge, this]
           {
               auto [left, right] = *edges.at(edge);
               return std::make_pair(left, right);
           });
}

void Topology::streamTo(std::ostream& os) const
{
    os << "lastVertexID = " << lastVertexID << ", "
       << "lastEdgeID = " << lastEdgeID << std::endl;

    os << "vertexIDs:" << std::endl;;

    for (const auto& id : vertices)
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
