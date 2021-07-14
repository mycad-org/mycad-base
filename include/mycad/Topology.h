#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include "detail/Topology.h"

#include <map>
#include <list>
#include <string>
#include <utility> // std::pair
#include <vector>
#include "tl/expected.hpp"

namespace mycad::topo
{
    struct VertexID
    {
        int index;

        auto operator<=>(VertexID const&) const = default;
    };

    struct EdgeID
    {
        int index;

        auto operator<=>(EdgeID const&) const = default;
    };

    struct Chain
    {
        VertexID vStart;
        EdgeID eStart;
    };

    //! used instead of std::optional so that we can compose it with other tl::expected
    using Error              = std::string;
    using VertexIDPair       = std::pair<VertexID, VertexID>;
    using EitherVertexIDPair = tl::expected<VertexIDPair, Error>;
    using EitherVertexID     = tl::expected<VertexID, Error>;
    using EitherEdgeID       = tl::expected<EdgeID, Error>;
    using EitherChain        = tl::expected<Chain, Error>;
    using EdgeIDs            = std::vector<EdgeID>;
    using EitherEdgeIDs      = tl::expected<EdgeIDs, Error>;

    /**
     *  Any method that returns a `tl::expected` has built-in error checking.
     *  This is useful for chaining together operations that may return an
     *  error, short-circuiting on the first error.
     *
     *  Each of these functions has an 'unsafe' version which does not perform
     *  the error-checking. You'll be responsible for making sure you call them
     *  appropriately.
     */
    class Topology
    {
        public:
            bool operator==(Topology const&) const = default;

            /** @brief checks if two topologies are mostly equivalent
             */
            auto similar(Topology const &other) const -> bool;

            auto hasVertex(VertexID v) const -> bool;
            auto hasEdge(EdgeID e) const -> bool;

            /** @brief A 'free' vertex does is not adajacent to anything
             */
            auto addFreeVertex() -> VertexID;

            /** @brief an Edge is always adjacent to exactly two Vertices
             */
            auto makeEdge(VertexID v1, VertexID v2) -> EitherEdgeID;
            auto unsafe_makeEdge(VertexID v1, VertexID v2) -> EdgeID;

            /** @brief creates a directional connection between two edges
             *  @returns error string if either edge doesn't exist in the
             *           topology
             *  @returns error string if a Chain from \param fromEdge to \param
             *           toEdge already exists
             *  @returns error string if the two Edge do not share a common
             *           Vertex
             */
            auto makeChain(EdgeID fromEdge, EdgeID toEdge) -> EitherChain;
            auto unsafe_makeChain(EdgeID fromEdge, EdgeID toEdge) -> Chain;

            /** @returns empty vector if valid vertex is 'free'
             *  @returns error sring if the vertex does not exist in the
             *           topology
             */
            auto edgesAdjacentToVertex(VertexID v) const -> EitherEdgeIDs;
            auto unsafe_edgesAdjacentToVertex(VertexID v) const -> EdgeIDs;

            /** @returns A pair `(left, right)` of vertex IDs corresponding to
             *           this Edge
             *  @returns error sring if the edge does not exist in the topology
             */
            auto getEdgeVertices(EdgeID edge) const -> EitherVertexIDPair;
            auto unsafe_getEdgeVertices(EdgeID edge) const -> VertexIDPair;

            /** @brief find the Vertex on the other side of the Edge
             *  @returns error string if either @v@ or @e@ does not exist in
             *           the topology
             *  @returns error string if the Vertex and Edge are not adjacent to
             *           each other
             */
            auto oppositeVertex(VertexID v, EdgeID e) const -> EitherVertexID;
            auto unsafe_oppositeVertex(VertexID v, EdgeID e) const -> VertexID;

            /** @brief returns all Edges in the Chain
             *  @returns error if the chain does not exist
             */
            auto getChainEdges(Chain chain) const -> EitherEdgeIDs;
            auto unsafe_getChainEdges(Chain chain) const -> EdgeIDs;

            /** @returns false if the Edge doesn't exist
             */
            auto deleteEdge(EdgeID e) -> bool;

            auto streamTo(std::ostream &os) const -> void;
        private:
            // std::vector::size can't be relied upon for UID's since when
            // items are deleted the size scales appropriately.
            int lastVertexID = 0;
            int lastEdgeID = 0;

            std::map<VertexID, detail::Vertex> vertices{};
            std::map<EdgeID, detail::Edge> edges{};
    };


    auto operator<<(std::ostream &os, VertexID const &v) -> std::ostream &;
    auto operator<<(std::ostream &os, EdgeID const &e) -> std::ostream &;
    auto operator<<(std::ostream &os, Topology const &topo) -> std::ostream &;
} // namespace mycad::topo


#endif // MYCAD_TOPOLOGY_HEADER
