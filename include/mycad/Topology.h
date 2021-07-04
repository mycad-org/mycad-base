#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
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

                bool operator==(const Topology&) const = default;
                /** @brief checks if two topologies are mostly equivalent
                 */
                bool similar(const Topology& other) const;

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
                edgesAdjacentToVertex(int) const;

                /** @returns A pair `(left, right)` of vertex IDs corresponding
                 *           to this Edge
                 *  @returns error sring if the edge does not exist in the
                 *           topology
                 */
                tl::expected<std::pair<int, int>, std::string>
                getEdgeVertices(int edge) const;

                /** @returns false if the Edge doesn't exist
                 */
                bool deleteEdge(int);

                void streamTo(std::ostream& os) const;
            private:
                int lastVertexID = 0;
                int lastEdgeID = 0;

                std::unordered_set<int> vertices;
                std::map<int, std::unique_ptr<detail::Edge>> edges;
        };


        std::ostream& operator<<(std::ostream& os, const Topology& topo);
    } // namespace topo
}     // namespace mycad


#endif // MYCAD_TOPOLOGY_HEADER
