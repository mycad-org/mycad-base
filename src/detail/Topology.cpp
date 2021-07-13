#include "mycad/detail/Topology.h"
#include "mycad/Topology.h"

#include <algorithm>

using namespace mycad::topo;

auto detail::hasVertex(const VertexID& v, const std::map<VertexID, Vertex>& vs)
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

auto detail::hasEdge(const EdgeID& edge, const std::map<EdgeID, Edge>& es)
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

auto detail::getCommonVertexID( const EdgeID& edge1, const EdgeID& edge2, const std::map<EdgeID, Edge>& es)
-> tl::expected<int, std::string>
{
    return
        hasEdge(edge1, es)
        .and_then(std::bind(hasEdge, edge2, es))
        .and_then([edge1, edge2, &es]() -> tl::expected<int, std::string>
        {
            auto [v1, v2] = es.at(edge1);
            auto [v3, v4] = es.at(edge2);

            if (v1 == v3)
            {
                return v1;
            }
            else if (v1 == v4)
            {
                return v2;
            }
            else if (v2 == v3)
            {
                return v2;
            }
            else if (v3 == v4)
            {
                return v4;
            }
            else
            {
                return tl::make_unexpected(
                    "The two edges with IDs " + std::to_string(edge1.index) +
                    " and " + std::to_string(edge2.index) +
                    " do not appear to share a common Vertex");
            }
        });
}

auto detail::getLink (VertexID v, EdgeID e, const std::map<VertexID, detail::Vertex>& vs)
-> tl::expected<detail::Link, std::string>
{
    auto links = vs.at(v).links;
    auto match = [&e](const detail::Link& link){return link.parentEdgeIndex == e.index;};
    auto ret   = std::ranges::find_if(links, match);
    if (ret == links.end())
    {
        return tl::make_unexpected(
            "There is no Link between Vertex with ID = " + std::to_string(v.index) +
            " and Edge with ID = " + std::to_string(e.index));
    }
    else
    {
        return *ret;
    }
}

auto detail::crawlLinks (const detail::Link& curLink, std::vector<EdgeID>& chain, const std::map<VertexID, detail::Vertex>& vs)
-> std::vector<EdgeID>&
{
    if(curLink.next)
    {
        const auto [nextVertex, nextEdge] = curLink.next.value();
        chain.push_back(EdgeID(nextEdge));

        auto nextLink = getLink(VertexID(nextVertex), EdgeID(nextEdge), vs);
        if(nextLink)
        {
            return crawlLinks(nextLink.value(), chain, vs);
        }
    }

    return chain;
}

