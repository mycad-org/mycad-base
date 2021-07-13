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
        std::optional<Link*> next = {};

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

} // mycad::topo::detail

#endif
