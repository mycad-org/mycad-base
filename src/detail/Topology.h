#ifndef TOPOLOGY_DETAIL_H
#define TOPOLOGY_DETAIL_H

#include "tl/expected.hpp"

#include <string>
#include <unordered_set>

namespace mycad
{
    namespace topo
    {
        namespace detail
        {
            tl::expected<void, std::string>
            hasVertex(int v, std::unordered_set<int> vs);

            struct Edge
            {
                int leftVertexID;
                int rightVertexID;
            };
        }
    } // namespace topo
} // namespace mycad
#endif // TOPOLOGY_DETAIL_H
