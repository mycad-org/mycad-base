#include "mycad/Topology.h"
#include "detail/Topology.h"

#include <algorithm>
#include <iostream>

using namespace mycad::topo;

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
auto Topology::similar(const Topology& other) const -> bool
{
    return vertices == other.vertices && edges == other.edges;
}

auto Topology::hasVertex(VertexID v) const -> bool
{
    return detail::hasVertex(v, vertices).has_value();
}

auto Topology::hasEdge(EdgeID e) const -> bool
{
    return detail::hasEdge(e, edges).has_value();
}

auto Topology::addFreeVertex() -> VertexID
{
    VertexID v(lastVertexID++);
    vertices.emplace(v, detail::Vertex{});
    return v;
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
auto Topology::makeEdge(VertexID v1, VertexID v2) -> EitherEdgeID
{
    if (not vertices.contains(v1))
    {
        return tl::unexpected(
            std::string("The vertex v1 = ") + std::to_string(v1.index) +
                        " does not exist in the topology");
    }
    else if (not vertices.contains(v2))
    {
        return tl::unexpected(
            std::string("The vertex v2 = ") + std::to_string(v2.index) +
                        " does not exist in the topology");
    }

    for (const auto& [key, edge] : edges)
    {
        auto [left, right] = edge;
        if ((left == v1.index && right == v2.index) ||
            (left == v2.index && right == v1.index))
        {
            return tl::unexpected(
                std::string("Only one Edge can exist between any two Vertices"));
        }
    }

    EdgeID edge(lastEdgeID++);
    edges.emplace(edge, detail::Edge{v1.index, v2.index});

    // Gather our data from storage
    detail::Vertex& leftVertex = vertices.at(v1);
    detail::Vertex& rightVertex = vertices.at(v2);

    // Update vertices with the appropriate links
    leftVertex.links.emplace_back(v1.index, edge.index);
    rightVertex.links.emplace_back(v2.index, edge.index);

    return edge;
}

auto Topology::unsafe_makeEdge(VertexID v1, VertexID v2) -> EdgeID
{
    return makeEdge(v1, v2).value();
}

auto Topology::deleteEdge(EdgeID edge) -> bool
{
    if (not detail::hasEdge(edge, edges))
    {
        return false;
    }
    else
    {
        // first delete the edge from the edges map
        edges.erase(edge);

        // Now we have to remove any links from the vertex map
        auto rm =
            [edge](const detail::Link& link)
                {
                    return link.parentEdgeIndex == edge.index;
                };

        std::ranges::for_each(vertices,
            [&rm](auto& pair)
            {
                detail::Vertex& vertex = pair.second;
                const auto rem = std::ranges::remove_if(vertex.links, rm); 
                vertex.links.erase(rem.begin(), rem.end());
            }
        );

        return true;
    }
}

auto Topology::makeChain(EdgeID fromEdge, EdgeID toEdge) -> Maybe
{
    return
        detail::getCommonVertexID(fromEdge, toEdge, std::cref(edges))
        .map([](int /*v*/) {
            return;
        });
}

auto Topology::edgesAdjacentToVertex(VertexID v) const -> EitherEdgeIDs
{
    return detail::hasVertex(v, vertices)
           .map([v, this]
           {
               std::vector<EdgeID> out;
               for (const auto& [key, edge]: edges)
               {
                   if (edge.leftVertexID == v.index || edge.rightVertexID == v.index)
                   {
                       out.push_back(key);
                   }
               }
               return out;
           });
}

auto Topology::unsafe_edgesAdjacentToVertex(VertexID v) const -> EdgeIDs
{
    return edgesAdjacentToVertex(v).value();
}

auto Topology::getEdgeVertices(EdgeID edge) const -> EitherVertexIDPair
{
    return detail::hasEdge(edge, edges)
           .map([edge, this]
           {
               auto [left, right] = edges.at(edge);
               return std::make_pair(VertexID(left), VertexID(right));
           });
}

auto Topology::unsafe_getEdgeVertices(EdgeID edge) const -> VertexIDPair
{
    return getEdgeVertices(edge).value();
}

auto Topology::oppositeVertex(VertexID v, EdgeID e) const -> EitherVertexID
{
    return
        detail::hasVertex(v, vertices)
        .and_then(std::bind(detail::hasEdge, e, std::cref(edges)))
        .and_then([v, e, this]() -> tl::expected<VertexID, std::string>
        {
            auto [left, right] = edges.at(e);
            if (left == v.index)
            {
                return VertexID(right);
            }
            else if (right == v.index )
            {
                return VertexID(left);
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

auto Topology::unsafe_oppositeVertex(VertexID v, EdgeID e) const -> VertexID
{
    return oppositeVertex(v, e).value();
}

auto Topology::getChainEdges(VertexID vertex, EdgeID edge) const -> EitherEdgeIDChain
{
    for (const auto& link : vertices.at(vertex).links)
    {
        if (link.parentEdgeIndex == edge.index)
        {

        }
    }
    return {};
}

auto Topology::streamTo(std::ostream& os) const -> void
{
    os << "lastVertexID = " << lastVertexID << ", "
       << "lastEdgeID = " << lastEdgeID << std::endl;

    os << "vertexIDs:" << std::endl;;

    for (const auto& [key, v] : vertices)
    {
        os << "    vid: " << key.index << std::endl;

        for (const auto& link : v.links)
        {
            os << "        link" << "\n"
               << "            parentVertex = " << link.parentVertexIndex << '\n'
               << "            parentEdge   = " << link.parentEdgeIndex << '\n';
        }
    }

    os << "edges:" << std::endl;
    for (const auto& [key, edge] : edges)
    {
        os << "    eid: " << key.index << ", "
           << "        leftVertexID = " << edge.leftVertexID << ", "
           << "        rightVertexID = " << edge.rightVertexID << std::endl;
    }
}

auto mycad::topo::operator<<(std::ostream& os, const VertexID& v) -> std::ostream&
{
    os << "V" << std::to_string(v.index);
    return os;
}

auto mycad::topo::operator<<(std::ostream& os, const EdgeID& e) -> std::ostream&
{
    os << "E" << std::to_string(e.index);
    return os;
}

auto mycad::topo::operator<<(std::ostream& os, const Topology& topo) -> std::ostream&
{
    topo.streamTo(os);
    return os;
}
