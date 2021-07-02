#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include <set>
#include <string>
#include "tl/expected.hpp"

namespace mycad {
    namespace topo {
        class Topology{
            public:
                int addFreeVertex();

                const std::set<int> getEdgeIDs();

                tl::expected<int, std::string> makeEdge(int v1, int v2);
            private:
                int lastVertexID = 0;
                int lastEdgeID = 0;

                std::set<int> vertexIDs;
                std::set<int> edgeIDs;
        };
    } // namespace topo
}     // namespace mycad


#endif // MYCAD_TOPOLOGY_HEADER
