#ifndef TOPOLOGY_DETAIL_H
#define TOPOLOGY_DETAIL_H

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
        }
    } // namespace topo
} // namespace mycad
#endif // TOPOLOGY_DETAIL_H
