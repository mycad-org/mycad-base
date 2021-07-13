#ifndef MYCAD_TOPOLOGY_DETAIL_HEADER
#define MYCAD_TOPOLOGY_DETAIL_HEADER

#include <optional>
#include <map>
#include <string>
#include <vector>

#include "tl/expected.hpp"

namespace mycad::topo
{
    struct VertexID;
    struct EdgeID;
} // namespace mycad::topo
namespace mycad::topo::detail
{
    struct Link
    {
        int parentVertexIndex = 0;
        int parentEdgeIndex = 0;
        // optional({nextVertexID, nextEdgeID}) - I couldn't find a reasonably
        // 'easy' way to make this optional(Link). I guess I could use a Link*
        // but I'd rather keep it simple like this for now.
        std::optional<std::pair<int, int>> next = {};

        auto operator<=>(Link const &other) const = default;
    };

    struct Vertex
    {
        std::vector<Link> links{};

        auto operator<=>(Vertex const &other) const = default;
    };

    struct Edge
    {
        int leftVertexID;
        int rightVertexID;

        auto operator<=>(Edge const &other) const = default;
    };

    auto hasVertex(VertexID const &v, std::map<VertexID, Vertex> const &vs)
    -> tl::expected<void, std::string>;

    auto hasEdge(EdgeID const &edge, std::map<EdgeID, Edge> const &es)
    -> tl::expected<void, std::string>;

    auto getCommonVertexID(EdgeID const &edge1, EdgeID const &edge2, std::map<EdgeID, Edge> const &es)
    -> tl::expected<int, std::string>;

    auto getLink (VertexID const v, EdgeID const e, std::map<VertexID, detail::Vertex> const &vs)
    -> tl::expected<detail::Link, std::string>;

    // recursive function to collect all EdgeID
    auto crawlLinks ( detail::Link const &curLink, std::vector<EdgeID> &chain, std::map<VertexID, detail::Vertex> const &vs)
    -> std::vector<EdgeID> &;
} // mycad::topo::detail

#endif
