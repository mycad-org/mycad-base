#include "mycad/Topology.h"
#include "detail/Topology.h"

#include <algorithm>
#include <iostream>

using namespace mycad::topo;

Topology::Topology() = default;

Topology::Topology(const Topology& other)
    : lastVertexID(other.lastVertexID),
      lastEdgeID(other.lastEdgeID),
      vertices(other.vertices),
      edges(other.edges)
{}

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
    this->edges = other.edges;

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

const Vertex& Topology::addFreeVertex()
{
    Vertex v = Vertex(lastVertexID++);
    vertices.push_back(v);
    return vertices.back();
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
tl::expected<Edge, std::string> Topology::makeEdge(Vertex v1, Vertex v2)
{
    // std::ranges::includes requires the range to be sorted
    std::vector<int> indices;
    std::ranges::transform(
        vertices,
        std::back_inserter(indices),
        [](const auto& v){return v.index;});
    std::ranges::sort(indices);
    auto check = std::vector<int>{v1.index, v2.index};
    std::ranges::sort(check);
    if (not std::ranges::includes(indices, check))
    {
        return tl::unexpected(
            std::string("One or both of v1 = ") + std::to_string(v1.index) +
                        " and v2 = " + std::to_string(v2.index) +
                        " do not exist in the topology");
    }

    for (const auto& edge : edges)
    {
        auto [left, right] = edge.getVertexIDs();
        if ((left == v1.index && right == v2.index) ||
            (left == v2.index && right == v1.index))
        {
            return tl::unexpected(
                std::string("Only one Edge can exist between any two Vertices"));
        }
    }

    Edge edge(lastEdgeID++, v1.index, v2.index);
    edges.push_back(edge);
    return edge;
}

bool Topology::deleteEdge(Edge edge)
{
    return
        std::erase_if(
            edges,
            [&edge](const Edge& check){return edge == check;}
        ) == 1;
}

tl::expected<void, std::string>
Topology::makeChain(Edge fromEdge, Edge toEdge)
{
    return
        detail::getCommonVertexID(fromEdge, toEdge, std::cref(edges))
        .map([](int /*v*/) {
            return;
        });
}

tl::expected<std::vector<Edge>, std::string>
Topology::edgesAdjacentToVertex(Vertex v) const
{
    return detail::hasVertex(v, vertices)
           .map([v, this]
           {
               std::vector<Edge> out;
               for (const auto& edge: edges)
               {
                   if (edge.leftVertexID == v.index || edge.rightVertexID == v.index)
                   {
                       out.push_back(edge);
                   }
               }
               return out;
           });
}

tl::expected<std::pair<Vertex, Vertex>, std::string>
Topology::getEdgeVertices(Edge edge) const
{
    return detail::hasEdge(edge, edges)
           .map([edge, this]
           {
               auto [left, right] = edges.at(edge.index).getVertexIDs();
               return std::make_pair(Vertex(left), Vertex(right));
           });
}

tl::expected<Vertex, std::string> Topology::oppositeVertex(Vertex v, Edge e) const
{
    return
        detail::hasVertex(v, vertices)
        .and_then(std::bind(detail::hasEdge, e, std::cref(edges)))
        .and_then([v, e, this]() -> tl::expected<Vertex, std::string>
        {
            auto [left, right] = edges.at(e.index).getVertexIDs();
            if (left == v.getIndex())
            {
                return vertices.at(right);
            }
            else if (right == v.index )
            {
                return vertices.at(left);
            }
            else
            {
                return tl::unexpected(
                       std::string("vertex with id = ") + std::to_string(v.index) +
                                   " is not adjacent to edge with id ="  +
                                   std::to_string(e.index));
            }
        });
}

tl::expected<std::list<Edge>, std::string>
Topology::getChainEdges(Vertex /*vertex*/, Edge /*edge*/) const
{
    return {};
}

void Topology::streamTo(std::ostream& os) const
{
    os << "lastVertexID = " << lastVertexID << ", "
       << "lastEdgeID = " << lastEdgeID << std::endl;

    os << "vertexIDs:" << std::endl;;

    for (const auto& v : vertices)
    {
        os << "    vid: " << v.index << std::endl;
    }

    os << "edges:" << std::endl;
    for (const auto& edge: edges)
    {
        os << "    eid: " << edge.index << ", "
           << "leftVertexID = " << edge.leftVertexID << ", "
           << "rightVertexID = " << edge.rightVertexID << std::endl;
    }
}

std::ostream& mycad::topo::operator<<(std::ostream& os, const Vertex& v)
{
    v.streamTo(os);
    return os;
}

std::ostream& mycad::topo::operator<<(std::ostream& os, const Edge& e)
{
    e.streamTo(os);
    return os;
}

std::ostream& mycad::topo::operator<<(std::ostream& os, const Topology& topo)
{
    topo.streamTo(os);
    return os;
}
