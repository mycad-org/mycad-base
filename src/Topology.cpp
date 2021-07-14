#include "mycad/Topology.h"

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
auto Topology::similar(Topology const &other) const -> bool
{
    const std::vector vals{
        vertices == other.vertices,
        vertices == other.vertices
    };

    auto isTrue = [](bool x){return x == true;};

    return std::ranges::all_of(vals, isTrue);
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
        return tl::make_unexpected(
            "The vertex v1 = " + std::to_string(v1.index) +
            " does not exist in the topology");
    }
    else if (not vertices.contains(v2))
    {
        return tl::make_unexpected(
            "The vertex v2 = " + std::to_string(v2.index) +
            " does not exist in the topology");
    }

    auto hasSameVertices = [&v1, &v2](auto pair)
    {
        auto const [index, edge] = pair;
        auto const [left, right] = edge;
        return (left == v1.index && right == v2.index) ||
               (left == v2.index && right == v1.index);
    };

    if(std::ranges::any_of(edges, hasSameVertices))
    {
        return tl::make_unexpected("Only one Edge can exist between any two Vertices");
    }

    EdgeID edge(lastEdgeID++);
    edges.emplace(edge, detail::Edge{v1.index, v2.index});

    // Gather our data from storage
    detail::Vertex &leftVertex = vertices.at(v1);
    detail::Vertex &rightVertex = vertices.at(v2);

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
        auto parentEdgeMatches =
            [edge](detail::Link const &link)
                {
                    return link.parentEdgeIndex == edge.index;
                };

        std::ranges::for_each(vertices,
            [&parentEdgeMatches](auto &pair)
            {
                detail::Vertex &vertex = pair.second;
                auto const rem = std::ranges::remove_if(vertex.links, parentEdgeMatches); 
                vertex.links.erase(rem.begin(), rem.end());
            }
        );

        return true;
    }
}

auto Topology::makeChain(EdgeID fromEdge, EdgeID toEdge) -> EitherChain
{
    return
        detail::getCommonVertexID(fromEdge, toEdge, std::cref(edges))
        .and_then([&toEdge, &fromEdge, this](VertexID const v) -> EitherChain
        {
            // This Vertex should have a link connected to the fromEdge
            auto fromLinkIndex = detail::getLinkIndex(v, fromEdge, vertices);
            if (!fromLinkIndex)
            {
                return tl::make_unexpected(fromLinkIndex.error());
            }

            // And one connected to the toEdge
            auto toLinkIndex = detail::getLinkIndex(v, toEdge, vertices);
            if (!toLinkIndex)
            {
                return tl::make_unexpected(toLinkIndex.error());
            }

            detail::Link &fromLink = vertices.at(v).links.at(fromLinkIndex.value());
            detail::Link const toLink = vertices.at(v).links.at(toLinkIndex.value());

            fromLink.next = {{toLink.parentVertexIndex, toLink.parentEdgeIndex}};

            VertexID fromVertex = oppositeVertex(v, fromEdge).value();
            return {Chain(fromVertex, fromEdge)};
        });
}

auto Topology::edgesAdjacentToVertex(VertexID v) const -> EitherEdgeIDs
{
    return
        detail::hasVertex(v, vertices)
        .map([v, this]()
        {
            auto vertexMatch =
                [v](auto const &pair)
                {
                    auto const &[key, edge] = pair;
                    return (edge.leftVertexID == v.index) ||
                           (edge.rightVertexID == v.index);
                };

            auto view =
                this->edges
                | std::views::filter(vertexMatch)
                | std::views::keys;

            return EdgeIDs(view.begin(), view.end());
        }
        );
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
               auto const [left, right] = edges.at(edge);
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
        .and_then(std::bind(detail::hasEdge, e, edges))
        .and_then(
        [vid = v.index, eid = e.index, edge = edges.at(e)]() -> EitherVertexID
        {
            auto const [left, right] = edge;
            if (left == vid)
            {
                return VertexID(right);
            }
            else if (right == vid)
            {
                return VertexID(left);
            }
            else
            {
                return tl::make_unexpected(
                    "vertex with id = " + std::to_string(vid) +
                    " is not adjacent to edge with id ="  + std::to_string(eid));
            }
        });
}

auto Topology::unsafe_oppositeVertex(VertexID v, EdgeID e) const -> VertexID
{
    return oppositeVertex(v, e).value();
}

auto Topology::getChainEdges(Chain chain) const -> EitherEdgeIDs
{
    auto [vertex, edge] = chain;
    auto const oppVertex = oppositeVertex(vertex, edge);
    if (!oppVertex)
    {
        return tl::make_unexpected(oppVertex.error());
    }

    auto const startLinkIndex = detail::getLinkIndex(oppVertex.value(), edge, vertices);

    if(startLinkIndex)
    {
        int i = startLinkIndex.value();
        EdgeIDs out{edge};

        // this is recursive
        detail::crawlLinks(vertices.at(oppVertex.value()).links.at(i), out, vertices);

        return out;
    }
    else
    {
        return tl::make_unexpected(startLinkIndex.error());
    }
}

auto Topology::unsafe_getChainEdges(Chain chain) const -> EdgeIDs
{
    return getChainEdges(chain).value();
}

auto Topology::streamTo(std::ostream &os) const -> void
{
    os << "lastVertexID = " << lastVertexID << ", "
       << "lastEdgeID = " << lastEdgeID << std::endl;

    os << "vertexIDs:" << std::endl;;

    for (auto const &[key, v] : vertices)
    {
        os << "    vid: " << key.index << std::endl;

        for (auto const &link : v.links)
        {
            os << "        link" << "\n"
               << "            parentVertex = " << link.parentVertexIndex << '\n'
               << "            parentEdge   = " << link.parentEdgeIndex << '\n';
            if (link.next)
            {
                auto const [nextVertex, nextEdge] = link.next.value();
                os << "            next = parentVertex = " << nextVertex << '\n'
                   << "                   parentEdge   = " << nextEdge << '\n';
            }
        }
    }

    os << "edges:" << std::endl;
    for (auto const &[key, edge] : edges)
    {
        os << "    eid: " << key.index << "\n"
           << "        leftVertexID = " << edge.leftVertexID << "\n"
           << "        rightVertexID = " << edge.rightVertexID << std::endl;
    }
}

auto mycad::topo::operator<<(std::ostream &os, VertexID const &v) -> std::ostream &
{
    os << "V" << std::to_string(v.index);
    return os;
}

auto mycad::topo::operator<<(std::ostream &os, EdgeID const &e) -> std::ostream &
{
    os << "E" << std::to_string(e.index);
    return os;
}

auto mycad::topo::operator<<(std::ostream &os, Topology const &topo) -> std::ostream &
{
    topo.streamTo(os);
    return os;
}
