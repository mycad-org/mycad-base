#ifndef MYCAD_TYPES_HEADER
#define MYCAD_TYPES_HEADER

#include <optional>
#include <vector>

namespace mycad
{
    class Line;

    using Lines        = std::vector<Line>;
    using VertexID     = std::size_t;
    using VertexIDPair = std::pair<VertexID, VertexID>;
    using EdgeID       = int;
    using EdgeIDs      = std::vector<EdgeID>;

    struct Chain
    {
        VertexID whichVertex;
        std::size_t whichLink;
    };

    using MaybeLine         = std::optional<Line>;
    using MaybeVertexID     = std::optional<VertexID>;
    using MaybeVertexIDPair = std::optional<VertexIDPair>;
    using MaybeEdgeID       = std::optional<EdgeID>;
    using MaybeEdgeIDs      = std::optional<EdgeIDs>;
    using MaybeChain        = std::optional<Chain>;

} // namespace mycad

#endif // MYCAD_TYPES_HEADER
