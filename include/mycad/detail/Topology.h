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

        auto operator<=>(const Link& other) const = default;
    };

    struct Vertex
    {
        std::vector<Link> links{};

        auto operator<=>(const Vertex& other) const = default;
    };

    struct Edge
    {
        int leftVertexID;
        int rightVertexID;

        auto operator<=>(const Edge& other) const = default;
    };

    auto hasVertex(const VertexID& v, const std::map<VertexID, Vertex>& vs)
    -> tl::expected<void, std::string>;

    auto hasEdge(const EdgeID& edge, const std::map<EdgeID, Edge>& es)
    -> tl::expected<void, std::string>;

    auto getCommonVertexID(const EdgeID& edge1, const EdgeID& edge2, const std::map<EdgeID, Edge>& es)
    -> tl::expected<int, std::string>;

    auto getLink (VertexID v, EdgeID e, const std::map<VertexID, detail::Vertex>& vs)
    -> tl::expected<detail::Link, std::string>;

    // recursive function to collect all EdgeID
    auto crawlLinks ( const detail::Link& curLink, std::vector<EdgeID>& chain, const std::map<VertexID, detail::Vertex>& vs)
    -> std::vector<EdgeID>&;
} // mycad::topo::detail

#endif
