#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include <map>
#include <memory>
#include <set>
#include <string>
#include "tl/expected.hpp"

namespace mycad
{
    namespace topo
    {
        namespace detail
        {
            struct Edge;
        }

        class Topology
        {
            public:
                Topology();
                Topology(const Topology& other);
                ~Topology();
                Topology& operator=(const Topology& other);

                int addFreeVertex();

                tl::expected<int, std::string> makeEdge(int v1, int v2);
            private:
                int lastVertexID = 0;
                int lastEdgeID = 0;

                std::set<int> vertexIDs;
                std::map<int, std::unique_ptr<detail::Edge>> edges;
        };
    } // namespace topo
}     // namespace mycad


#endif // MYCAD_TOPOLOGY_HEADER
