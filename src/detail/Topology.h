#ifndef TOPOLOGY_DETAIL_H
#define TOPOLOGY_DETAIL_H

#include "mycad/Topology.h"
#include "tl/expected.hpp"

#include <string>
#include <vector>

namespace mycad
{
    namespace topo
    {
        namespace detail
        {
            tl::expected<void, std::string>
            hasVertex(const Vertex& v, const std::vector<Vertex>& vs);

            tl::expected<void, std::string>
            hasEdge(const Edge& edge, const std::vector<Edge>& es);

            tl::expected<int, std::string>
            getCommonVertexID(const Edge& edge1, const Edge& edge2,
                              const std::vector<Edge>& es);
        } // namespace detail
    } // namespace topo
} // namespace mycad
#endif // TOPOLOGY_DETAIL_H
