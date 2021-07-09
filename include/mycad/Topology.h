#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include <list>
#include <string>
#include <utility> // std::pair
#include <vector>
#include "tl/expected.hpp"

namespace mycad
{
    namespace topo
    {
        class Vertex;
        class Edge;
        class Topology;

        struct Link
        {
            Vertex* parentVertex;
            Edge* parentEdge;

            bool operator==(const Link& other) const;
        };

        class Vertex
        {
            public:
                friend Topology;

                explicit Vertex(int v);
                bool operator< (const Vertex& other) const;
                bool operator== (const Vertex& other) const;

                int getIndex() const;

                void streamTo(std::ostream& os) const;

            private:
                int index;
                std::vector<Link> links;
        };

        class Edge
        {
            public:
                friend Topology;

                explicit Edge(int e, int l, int r);
                bool operator==(const Edge&) const = default;

                int getIndex() const;
                std::pair<int, int> getVertexIDs() const;

                void streamTo(std::ostream& os) const;

            private:
                int index;
                int leftVertexID;
                int rightVertexID;
        };

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
                const Vertex& addFreeVertex();

                /** @brief an Edge is always adjacent to exactly two Vertices
                 */
                tl::expected<Edge, std::string>
                makeEdge(Vertex v1, Vertex v2);

                /** @brief creates a directional connection between two edges
                 *  @returns error string if either edge doesn't exist in the
                 *           topology
                 *  @returns error string if a Chain from \param fromEdge to
                 *           \param toEdge already exists
                 *  @returns error string if the two Edge do not share a common
                 *           Vertex
                 */
                tl::expected<void, std::string>
                makeChain(Edge /*fromEdge*/, Edge /*toEdge*/);

                /** @returns empty vector if valid vertex is 'free'
                 *  @returns error sring if the vertex does not exist in the
                 *           topology
                 */
                tl::expected<std::vector<Edge>, std::string>
                edgesAdjacentToVertex(Vertex v) const;

                /** @returns A pair `(left, right)` of vertex IDs corresponding
                 *           to this Edge
                 *  @returns error sring if the edge does not exist in the
                 *           topology
                 */
                tl::expected<std::pair<Vertex, Vertex>, std::string>
                getEdgeVertices(Edge edge) const;

                /** @brief find the Vertex on the other side of the Edge
                 *  @returns error string if either @v@ or @e@ does not exist in
                 *           the topology
                 *  @returns error string if the Vertex and Edge are not
                 *           adjacent to each other
                 */
                tl::expected<Vertex, std::string> oppositeVertex(Vertex v, Edge e) const;

                /** @brief returns all Edges in the Chain
                 *
                 *  The Edge are returns as a std::list to maintain their order
                 *
                 *  @returns error if the chain does not exist
                 */
                tl::expected<std::list<Edge>, std::string>
                getChainEdges(Vertex /*vertex*/, Edge /*edge*/) const;

                /** @returns false if the Edge doesn't exist
                 */
                bool deleteEdge(Edge e);

                void streamTo(std::ostream& os) const;
            private:
                int lastVertexID = 0;
                int lastEdgeID = 0;

                std::vector<Vertex> vertices{};
                std::vector<Edge> edges{};
        };


        std::ostream& operator<<(std::ostream& os, const Vertex& v);
        std::ostream& operator<<(std::ostream& os, const Edge& e);
        std::ostream& operator<<(std::ostream& os, const Topology& topo);
    } // namespace topo
}     // namespace mycad


#endif // MYCAD_TOPOLOGY_HEADER
