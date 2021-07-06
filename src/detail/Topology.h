#ifndef TOPOLOGY_DETAIL_H
#define TOPOLOGY_DETAIL_H

#include "tl/expected.hpp"

#include <map>
#include <memory>
#include <string>
#include <unordered_set>

namespace mycad
{
    namespace topo
    {
        namespace detail
        {
            struct Edge
            {
                int leftVertexID;
                int rightVertexID;
            };

            tl::expected<void, std::string>
            hasVertex(int v, std::unordered_set<int> vs);

            tl::expected<void, std::string>
            hasEdge(int edge, const std::map<int, std::unique_ptr<Edge>>& es);

            tl::expected<int, std::string>
            getCommonVertex(int edge1, int edge2,
                            const std::map<int, std::unique_ptr<Edge>>& es);
        }
    } // namespace topo
} // namespace mycad
#endif // TOPOLOGY_DETAIL_H
