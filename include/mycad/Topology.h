#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include "detail/Topology.h"

#include <map>
#include <list>
#include <string>
#include <utility> // std::pair
#include <vector>
#include "tl/expected.hpp"

namespace mycad
{
    namespace topo
    {
        struct VertexID
        {
            int index;

            auto operator<=>(const VertexID&) const = default;
        };

        struct EdgeID
        {
            int index;

            auto operator<=>(const EdgeID&) const = default;
        };

        class Topology
        {
            public:
                bool operator==(const Topology&) const = default;

                /** @brief checks if two topologies are mostly equivalent
                 */
                bool similar(const Topology& other) const;

                /** @brief A 'free' vertex does is not adajacent to anything
                 */
                VertexID addFreeVertex();

                /** @brief an Edge is always adjacent to exactly two Vertices
                 */
                tl::expected<EdgeID, std::string>
                makeEdge(VertexID v1, VertexID v2);

                /** @brief creates a directional connection between two edges
                 *  @returns error string if either edge doesn't exist in the
                 *           topology
                 *  @returns error string if a Chain from \param fromEdge to
                 *           \param toEdge already exists
                 *  @returns error string if the two Edge do not share a common
                 *           Vertex
                 */
                tl::expected<void, std::string>
                makeChain(EdgeID /*fromEdge*/, EdgeID /*toEdge*/);

                /** @returns empty vector if valid vertex is 'free'
                 *  @returns error sring if the vertex does not exist in the
                 *           topology
                 */
                tl::expected<std::vector<EdgeID>, std::string>
                edgesAdjacentToVertex(VertexID v) const;

                /** @returns A pair `(left, right)` of vertex IDs corresponding
                 *           to this Edge
                 *  @returns error sring if the edge does not exist in the
                 *           topology
                 */
                tl::expected<std::pair<VertexID, VertexID>, std::string>
                getEdgeVertices(EdgeID edge) const;

                /** @brief find the Vertex on the other side of the Edge
                 *  @returns error string if either @v@ or @e@ does not exist in
                 *           the topology
                 *  @returns error string if the Vertex and Edge are not
                 *           adjacent to each other
                 */
                tl::expected<VertexID, std::string>
                oppositeVertex(VertexID v, EdgeID e) const;

                /** @brief returns all Edges in the Chain
                 *
                 *  The Edge are returns as a std::list to maintain their order
                 *
                 *  @returns error if the chain does not exist
                 */
                tl::expected<std::list<EdgeID>, std::string>
                getChainEdges(VertexID /*vertex*/, EdgeID /*edge*/) const;

                /** @returns false if the Edge doesn't exist
                 */
                bool deleteEdge(EdgeID e);

                void streamTo(std::ostream& os) const;
            private:
                // std::vector::size can't be relied upon for UID's since when
                // items are deleted the size scales appropriately.
                int lastVertexID = 0;
                int lastEdgeID = 0;

                std::map<VertexID, detail::Vertex> vertices{};
                std::map<EdgeID, detail::Edge> edges{};
        };


        std::ostream& operator<<(std::ostream& os, const VertexID& v);
        std::ostream& operator<<(std::ostream& os, const EdgeID& e);
        std::ostream& operator<<(std::ostream& os, const Topology& topo);
    } // namespace topo
}     // namespace mycad


#endif // MYCAD_TOPOLOGY_HEADER
