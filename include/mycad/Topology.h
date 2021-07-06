#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include <list>
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

                /** @brief creates a directional connection between two edges
                 *  @returns error string if either edge doesn't exist in the
                 *           topology
                 *  @returns error string if a Chain from \param fromEdge to
                 *           \param toEdge already exists
                 *  @returns error string if the two Edge do not share a common
                 *           Vertex
                 */
                tl::expected<void, std::string>
                makeChain(int /*fromEdge*/, int /*toEdge*/){return {};}

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

                /** @brief find the Vertex on the other side of the Edge
                 *  @returns error string if either @v@ or @e@ does not exist in
                 *           the topology
                 *  @returns error string if the Vertex and Edge are not
                 *           adjacent to each other
                 */
                tl::expected<int, std::string> oppositeVertex(int v, int e) const;

                /** @brief returns all Edges in the Chain
                 *
                 *  The Edge are returns as a std::list to maintain their order
                 *
                 *  @returns error if the chain does not exist
                 */
                tl::expected<std::list<int>, std::string>
                getChainEdges(int /*vertex*/, int /*edge*/) const{return {};}

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
