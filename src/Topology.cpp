#include "mycad/Topology.h"

#include <algorithm>
#include <iostream>
#include <optional>

using namespace mycad;
namespace ranges = std::ranges;
namespace views = std::views;

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

    return ranges::all_of(vals, isTrue);
}

auto Topology::hasVertex(VertexID v) const -> bool
{
    if (v >= vertices.size() || v == InvalidVertexID)
    {
        return false;
    }
    else
    {
        return true;
    }
}

auto Topology::hasEdge(EdgeID e) const -> bool
{
    return edges.count(e) == 1;
}

auto Topology::hasChain(Chain c) const -> bool
{
    auto maybeEdges = getChainEdges(c);
    return maybeEdges.has_value() ? maybeEdges.value().size() : false;
}

auto Topology::addFreeVertex() -> VertexID
{
    return vertices.emplace_back(vertices.size()).index.value();
}

/**
 * The two Vertices **can** be the same, in which case the Edge would be
 * considered a "loop" edge.
 *
 * An invalid EdgeID is returned if:
 *
 * 1. either or both vertices don't exist in the topology
 * 2. an Edge already exists between v1 and v2
 */
auto Topology::makeEdge(VertexID v1, VertexID v2) -> MaybeEdgeID
{
    if (not (hasVertex(v1) && hasVertex(v2)))
    {
        return std::nullopt;
    }

    auto hasSameVertices = [&v1, &v2](auto pair)
    {
        auto const [left, right] = pair.second.ends;
        return (left == v1 && right == v2) ||
               (left == v2 && right == v1);
    };

    if(ranges::any_of(edges, hasSameVertices))
    {
        return std::nullopt;
    }

    EdgeID edge(lastEdgeID++);
    edges.emplace(edge, detail::Edge{{v1, v2}});

    // Gather our data from storage
    detail::Vertex &leftVertex = vertices.at(v1);
    detail::Vertex &rightVertex = vertices.at(v2);

    // Update vertices with the appropriate links
    leftVertex.links.emplace_back(v1, edge);
    rightVertex.links.emplace_back(v2, edge);

    return edge;
}

auto Topology::deleteEdge(EdgeID edge) -> bool
{
    if (not hasEdge(edge))
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
                    return link.parentEdge == edge;
                };

        ranges::for_each(vertices,
            [&parentEdgeMatches](auto &vertex)
            {
                auto const rem = ranges::remove_if(vertex.links, parentEdgeMatches); 
                vertex.links.erase(rem.begin(), rem.end());
            }
        );

        return true;
    }
}

auto linkedToEdge(EdgeID const e)
{
    return [e](detail::Link const l)
           {return l.parentEdge == e;};
}

auto Topology::joinEdges(EdgeID fromEdge, EdgeID toEdge) -> Chain
{
    auto const v = detail::getCommonVertexID(fromEdge, toEdge, edges);

    // getCommonVertexID already checked if the edges belong to the topology
    if (not hasVertex(v))
    {
        return InvalidChain;
    }

    auto &links = vertices.at(v).links;

    // Any Edge can only be used **once** as a fromEdge or toEdge
    if (isFromEdge(fromEdge, links) || isToEdge(toEdge, links))
    {
        return InvalidChain;
    }

    // The common Vertex should have a Link to the fromEdge
    auto fromLinkIt = ranges::find_if(links, linkedToEdge(fromEdge));
    if (fromLinkIt == links.end())
    {
        return InvalidChain;
    }

    // And one connected to the toEdge
    auto const toLinkIt = ranges::find_if(links, linkedToEdge(toEdge));
    if (toLinkIt == links.end())
    {
        return InvalidChain;
    }

    fromLinkIt->next = {{toLinkIt->parentVertex, toLinkIt->parentEdge}};

    return {Chain(v, fromLinkIt - links.begin())};
}

auto Topology::joinEdges(MaybeEdgeID fromEdge, MaybeEdgeID toEdge) -> Chain
{
    if (not (fromEdge.has_value() && toEdge.has_value()))
    {
        return InvalidChain;
    }
    return joinEdges(*fromEdge, *toEdge);
}

auto Topology::extendChain(Chain c, EdgeID nextEdge) -> bool
{
    if (not (hasChain(c) || hasEdge(nextEdge)))
    {
        return false;
    }

    auto maybeEdges = getChainEdges(c);

    if(not maybeEdges.has_value())
    {
        return false;
    }

    auto const lastEdge = maybeEdges.value().back();

    return hasChain(joinEdges(lastEdge, nextEdge));
}

auto Topology::edgesAdjacentToVertex(VertexID v) const -> MaybeEdgeIDs
{
    if (not hasVertex(v))
    {
        return std::nullopt;
    }

    auto vertexMatch =
        [v](auto const &pair)
        {
            auto const &[key, e] = pair;
            return (e.ends.first == v) || (e.ends.second == v);
        };

    auto view =
        this->edges
        | views::filter(vertexMatch)
        | views::keys;

    return EdgeIDs(view.begin(), view.end());
}

auto Topology::getEdgeVertices(EdgeID edge) const -> MaybeVertexIDPair
{
    if (not hasEdge(edge))
    {
        return std::nullopt;
    }

   auto const [left, right] = edges.at(edge).ends;
   return std::make_pair(left, right);
}

auto Topology::getEdgeVertices(MaybeEdgeID edge) const -> MaybeVertexIDPair
{
    return edge.has_value() ? getEdgeVertices(*edge) : std::nullopt;
}

auto Topology::oppositeVertex(VertexID vid, EdgeID e) const -> MaybeVertexID
{
    if (not (hasVertex(vid) && hasEdge(e)))
    {
        return std::nullopt;
    }

    auto const [left, right] = edges.at(e).ends;

    if (left == vid)
    {
        return right;
    }
    else if (right == vid)
    {
        return left;
    }
    else
    {
        return std::nullopt;
    }
}

auto Topology::getChainEdges(Chain chain) const -> MaybeEdgeIDs
{
    auto [vertex, whichlink] = chain;
    if (not hasVertex(vertex))
    {
        return std::nullopt;
    }

    auto links = vertices.at(vertex).links;
    if (whichlink >= links.size())
    {
        return std::nullopt;
    }

    auto link = links.at(whichlink);

    EdgeIDs out{};

    while(link.next)
    {
        out.push_back({link.parentEdge});

        auto [chainVertex, chainEdge] = link.next.value();

        auto oppVertex = oppositeVertex(chainVertex, chainEdge);
        if(oppVertex)
        {
            links = vertices.at(*oppVertex).links;

            link = *std::ranges::find_if(links, linkedToEdge(chainEdge));
        }
        else
        {
            break;
        }
    }

    // get the last one
    if (out.size() > 0)
    {
        out.push_back({link.parentEdge});
    }

    return out;
}

auto Topology::streamTo(std::ostream &os) const -> void
{
    os << "lastVertexID = " << lastVertexID << ", "
       << "lastEdgeID = " << lastEdgeID << std::endl;

    os << "vertexIDs:" << std::endl;;

    for (VertexID i = 0; i < vertices.size(); i++)
    {
        detail::Vertex const & vertex = vertices.at(i);
        os << "    vid: " << i << std::endl;

        for (auto const &link : vertex.links)
        {
            os << "        link" << "\n"
               << "            parentVertex = " << link.parentVertex << '\n'
               << "            parentEdge   = " << link.parentEdge << '\n';
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
        os << "    eid: " << key << "\n"
           << "        leftVertexID = " << edge.ends.first << "\n"
           << "        rightVertexID = " << edge.ends.second << std::endl;
    }
}

/* auto mycad::operator<<(std::ostream &os, VertexID const &v) -> std::ostream & */
/* { */
/*     os << "V" << std::to_string(v); */
/*     return os; */
/* } */

auto operator<<(std::ostream &os, std::optional<std::size_t> const &val) -> std::ostream &
{
    if(val.has_value())
    {
        os << std::to_string(*val);
    }
    else
    {
        os << std::string("OptionalIsEmpty");
    }
    return os;
}

auto mycad::operator<<(std::ostream &os, Chain const &c) -> std::ostream &
{
    os << "Chain: " << c.whichVertex << ", Link Number " << c.whichLink;
    return os;
}

auto mycad::operator<<(std::ostream &os, Topology const &topo) -> std::ostream &
{
    topo.streamTo(os);
    return os;
}
