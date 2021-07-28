#ifndef MYCAD_TYPES_HEADER
#define MYCAD_TYPES_HEADER

#include <optional>
#include <vector>

namespace mycad
{
    class Line;

    using VertexID = std::size_t;
    using EdgeID   = int;

    using MaybeLine     = std::optional<Line>;
    using MaybeVertexID = std::optional<VertexID>;

    struct Chain
    {
        VertexID whichVertex;
        std::size_t whichLink;
    };

    constexpr VertexID InvalidVertexID = -1;
    constexpr EdgeID   InvalidEdgeID   = -1;
    constexpr Chain    InvalidChain{InvalidVertexID, 0};

    using VertexIDPair       = std::pair<VertexID, VertexID>;
    using EdgeIDs            = std::vector<EdgeID>;

} // namespace mycad

#endif // MYCAD_TYPES_HEADER
