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

auto detail::getCommonVertexID(EdgeID const &edge1, EdgeID const &edge2, std::map<EdgeID, Edge> const &es)
-> tl::expected<VertexID, std::string>
{
    return
        hasEdge(edge1, es)
        .and_then(std::bind(hasEdge, edge2, es))
        .and_then([edge1, edge2, &es]() -> tl::expected<VertexID, std::string>
        {
            auto [v1, v2] = es.at(edge1);
            auto [v3, v4] = es.at(edge2);

            if (v1 == v3)
            {
                return {{v1}};
            }
            else if (v1 == v4)
            {
                return {{v1}};
            }
            else if (v2 == v3)
            {
                return {{v2}};
            }
            else if (v3 == v4)
            {
                return {{v3}};
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

auto detail::getLinkIndex(VertexID v, EdgeID e, std::map<VertexID, detail::Vertex> const &vs)
-> tl::expected<std::vector<Link>::size_type, std::string>
{
    auto links = vs.at(v).links;
    auto match = [&e](detail::Link const &link){return link.parentEdgeIndex == e.index;};
    auto ret   = std::ranges::find_if(links, match);
    if (ret == links.end())
    {
        return tl::make_unexpected(
            "There is no Link between Vertex with ID = " + std::to_string(v.index) +
            " and Edge with ID = " + std::to_string(e.index));
    }
    else
    {
        return ret - links.begin();
    }
}

auto detail::crawlLinks (detail::Link const &curLink, std::vector<EdgeID> &chain, std::map<VertexID, detail::Vertex> const &vs)
-> std::vector<EdgeID> &
{
    if(curLink.next)
    {
        auto const [nextVertex, nextEdge] = curLink.next.value();
        chain.push_back(EdgeID(nextEdge));

        auto nextLinkIndex = getLinkIndex(VertexID(nextVertex), EdgeID(nextEdge), vs);
        if(nextLinkIndex)
        {
            detail::Link const &nextLink = vs.at(VertexID(nextVertex)).links.at(nextLinkIndex.value());
            return crawlLinks(nextLink, chain, vs);
        }
    }

    return chain;
}

