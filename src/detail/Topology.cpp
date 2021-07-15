#include "mycad/detail/Topology.h"
#include "mycad/Topology.h"

#include <algorithm>

using namespace mycad::topo;

auto detail::hasVertex(VertexID const &v, std::map<VertexID, Vertex> const &vs)
-> tl::expected<void, std::string>
{
    if (vs.contains(v) == 0)
    {
        return tl::make_unexpected(
            "Vertex with ID=" + std::to_string(v.index) + " not found"
        );
    }

    return {};
}

auto detail::hasEdge(EdgeID const &edge, std::map<EdgeID, Edge> const &es)
-> tl::expected<void, std::string>
{
    if (es.count(edge) == 1)
    {
        return {};
    }
    else
    {
        return tl::make_unexpected(
            "Edge with ID=" + std::to_string(edge.index) + " not found"
        );
    }
}

auto detail::getCommonVertexID(EdgeID const edge1, EdgeID const edge2,
                               std::map<EdgeID, Edge> const &es) -> VertexID
{
    auto pair1 = es.find(edge1);
    auto pair2 = es.find(edge2);
    if (pair1 == es.end() || pair2 == es.end())
    {
        return {-1};
    }

    auto [v1, v2] = pair1->second;
    auto [v3, v4] = pair2->second;

    if (v1 == v3)
    {
        return {v1};
    }
    else if (v1 == v4)
    {
        return {v1};
    }
    else if (v2 == v3)
    {
        return {v2};
    }
    else if (v3 == v4)
    {
        return {v3};
    }
    else
    {
        return {-1};
    }
}

auto detail::linkedToEdge(EdgeID const e)
{
    return [e](detail::Link const l)
           {return l.parentEdgeIndex == e.index;};
}


auto detail::crawlLinks (detail::Link const &curLink, std::vector<EdgeID> &chain,
                         std::map<VertexID, detail::Vertex> const &vs)
-> std::vector<EdgeID> &
{
    if(curLink.next)
    {
        auto const [nextVertex, nextEdge] = curLink.next.value();
        chain.push_back(EdgeID(nextEdge));

        auto const &links = vs.at({nextVertex}).links;
        auto nextLinkIt = std::ranges::find_if(links, linkedToEdge(EdgeID(nextEdge)));
        if(nextLinkIt != links.end())
        {
            return crawlLinks(*nextLinkIt, chain, vs);
        }
    }

    return chain;
}

