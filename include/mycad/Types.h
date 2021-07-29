#ifndef MYCAD_TYPES_HEADER
#define MYCAD_TYPES_HEADER

#include <optional>
#include <vector>

namespace mycad
{
    class Line;

    using VertexID = std::size_t;
    using EdgeID   = int;
    using EdgeIDs  = std::vector<EdgeID>;

    using MaybeLine     = std::optional<Line>;
    using MaybeVertexID = std::optional<VertexID>;
    using MaybeEdgeID   = std::optional<EdgeID>;
    using MaybeEdgeIDs  = std::optional<EdgeIDs>;

    struct Chain
    {
        VertexID whichVertex;
        std::size_t whichLink;
    };

    constexpr VertexID InvalidVertexID = -1;
    constexpr EdgeID   InvalidEdgeID   = -1;
    constexpr Chain    InvalidChain{InvalidVertexID, 0};

    using VertexIDPair       = std::pair<VertexID, VertexID>;

} // namespace mycad

#endif // MYCAD_TYPES_HEADER
