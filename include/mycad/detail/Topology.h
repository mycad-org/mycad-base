#ifndef MYCAD_TOPOLOGY_DETAIL_HEADER
#define MYCAD_TOPOLOGY_DETAIL_HEADER

#include <optional>
#include <map>
#include <string>
#include <vector>

#include "tl/expected.hpp"

namespace mycad
{
    namespace topo
    {
        struct VertexID;
        struct EdgeID;

        namespace detail
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

            tl::expected<void, std::string>
            hasVertex(const VertexID& v,
                      const std::map<VertexID, Vertex>& vs);

            tl::expected<void, std::string>
            hasEdge(const EdgeID& edge,
                    const std::map<EdgeID, Edge>& es);

            tl::expected<int, std::string>
            getCommonVertexID(const EdgeID& edge1,
                              const EdgeID& edge2,
                              const std::map<EdgeID, Edge>& es);
        } // detail
    } // topo
} // mycad

#endif
