#ifndef MYCAD_TOPOLOGY_DETAIL_HEADER
#define MYCAD_TOPOLOGY_DETAIL_HEADER

#include <functional>
#include <optional>
#include <map>
#include <string>
#include <vector>

#include "mycad/Types.h"

namespace mycad::detail
{
    struct Link
    {
        VertexID parentVertex = 0;
        EdgeID   parentEdge   = 0;
        // optional({nextVertexID, nextEdgeID}) - I couldn't find a reasonably
        // 'easy' way to make this optional(Link). I guess I could use a Link*
        // but I'd rather keep it simple like this for now.
        std::optional<std::pair<VertexID, EdgeID>> next = {};

        auto operator<=>(Link const &other) const = default;
    };

    struct Vertex
    {
        // no value means invalid Vertex
        std::optional<VertexID> index = std::nullopt;
        std::vector<Link> links{};

        auto operator<=>(Vertex const &other) const = default;
    };

    struct Edge
    {
        VertexIDPair ends;

        auto operator<=>(Edge const &other) const = default;
    };

    using Vertices = std::vector<Vertex>;

    auto getCommonVertexID(EdgeID const edge1, EdgeID const edge2,
                           std::map<EdgeID, Edge> const &es) -> VertexID;

    auto linkedToEdge(EdgeID const e);
    auto isFromEdge(EdgeID const fromEdge,
                    std::vector<Link> const &commonVertexLinks) -> bool;
    auto isToEdge  (EdgeID const fromEdge,
                    std::vector<Link> const &commonVertexLinks) -> bool;
} // mycad::detail

#endif
