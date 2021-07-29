#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include "mycad/Types.h"
#include "detail/Topology.h"

#include <map>
#include <list>
#include <string>
#include <utility> // std::pair
#include <vector>

namespace mycad
{
    class Topology
    {
        public:
            bool operator==(Topology const&) const = default;

            /** @brief checks if two topologies are mostly equivalent
             */
            auto similar(Topology const &other) const -> bool;

            auto hasVertex(VertexID v) const -> bool;
            auto hasEdge(EdgeID e) const -> bool;
            auto hasChain(Chain c) const -> bool;

            /** @brief A 'free' vertex does is not adajacent to anything
             */
            auto addFreeVertex() -> VertexID;

            /** @brief an Edge is always adjacent to exactly two Vertices
             */
            auto makeEdge(VertexID v1, VertexID v2) -> MaybeEdgeID;

            /** @brief creates a directional connection between two edges
             *  @returns an invalid Chain if:
             *      1. either Edge does not exist in the Toploogy
             *      2. the two Edge do not share a common vertex
             */
            auto joinEdges(EdgeID fromEdge, EdgeID toEdge) -> Chain;

            /** @brief a convenience to directly use return-value from makeEdge
             */
            auto joinEdges(MaybeEdgeID fromEdge, MaybeEdgeID toEdge) -> Chain;

            /** returns false if:
             *      1. the Chain or Edge provided are invalid in the topology
             *      2. The Edge provided does not have a common Vertex with the
             *         last Edge in the existing Chain
             *      3. The provided Chain is alread closed
             */
            auto extendChain(Chain c, EdgeID nextEdge) -> bool;

            /** @returns empty vector if valid vertex is 'free'
             *  @returns error sring if the vertex does not exist in the
             *           topology
             */
            auto edgesAdjacentToVertex(VertexID v) const -> MaybeEdgeIDs;

            /** @returns A pair `(left, right)` of vertex IDs corresponding to
             *           this Edge
             *  @returns invalid Vertices if the provided Edge does not exist in
             *           the topology
             */
            auto getEdgeVertices(EdgeID edge) const -> VertexIDPair;

            /** @brief a convenience to directly use the output of makeEdge
             */
            auto getEdgeVertices(MaybeEdgeID edge) const -> VertexIDPair;

            /** @brief find the Vertex on the other side of the Edge
             *  @returns invalid Vertex if:
             *      1. The provided Vertex does not exist in the topology
             *      2. The provided Edge does not exist in the topology
             *      3. The Vertex and Edge are not adjacent
             */
            auto oppositeVertex(VertexID v, EdgeID e) const -> MaybeVertexID;

            /** @brief returns all Edges in the Chain
             */
            auto getChainEdges(Chain chain) const -> MaybeEdgeIDs;

            /** @returns false if the Edge doesn't exist
             */
            auto deleteEdge(EdgeID e) -> bool;

            auto streamTo(std::ostream &os) const -> void;
        private:
            // std::vector::size can't be relied upon for UID's since when
            // items are deleted the size scales appropriately.
            int lastVertexID = 0;
            int lastEdgeID = 0;

            detail::Vertices vertices{};
            std::map<EdgeID, detail::Edge> edges{};
    };


    /* auto operator<<(std::ostream &os, VertexID const &v) -> std::ostream &; */
    auto operator<<(std::ostream &os, Chain const &c) -> std::ostream &;
    auto operator<<(std::ostream &os, Topology const &topo) -> std::ostream &;
} // namespace mycad::topo

auto operator<<(std::ostream &os, std::optional<std::size_t> const &val) -> std::ostream &;
#endif // MYCAD_TOPOLOGY_HEADER
