#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include <map>
#include <memory>
#include <unordered_set>
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

                /** @brief A 'free' vertex does is not adajacent to anything
                 */
                int addFreeVertex();

                /** @brief an Edge is always adjacent to exactly two Vertices
                 */
                tl::expected<int, std::string> makeEdge(int v1, int v2);

                /** @returns empty set if valid vertex is 'free'
                 *  @returns error sring if the vertex does not exist in the
                 *           topology
                 */
                tl::expected<std::unordered_set<int>, std::string>
                edgesAdjacentToVertex(int) const {return {};}

            private:
                int lastVertexID = 0;
                int lastEdgeID = 0;

                std::unordered_set<int> vertexIDs;
                std::map<int, std::unique_ptr<detail::Edge>> edges;
        };
    } // namespace topo
}     // namespace mycad


#endif // MYCAD_TOPOLOGY_HEADER
