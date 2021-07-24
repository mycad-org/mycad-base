#ifndef MYCAD_TOPOLOGY_DETAIL_HEADER
#define MYCAD_TOPOLOGY_DETAIL_HEADER

#include <optional>
#include <map>
#include <string>
#include <vector>

#include "mycad/Types.h"

namespace mycad
{
    struct EdgeID;
} // namespace mycad

namespace mycad::detail
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

    auto getCommonVertexID(EdgeID const edge1, EdgeID const edge2,
                           std::map<EdgeID, Edge> const &es) -> VertexID;

    auto linkedToEdge(EdgeID const e);
    auto isFromEdge(EdgeID const fromEdge,
                    std::vector<Link> const &commonVertexLinks) -> bool;
    auto isToEdge  (EdgeID const fromEdge,
                    std::vector<Link> const &commonVertexLinks) -> bool;
} // mycad::detail

#endif
