#include "mycad/Topology.h"
#include "tl/expected.hpp"

#include <catch2/catch.hpp>
#include "rapidcheck.h"
#include "rapidcheck/catch.h"

#include <algorithm>
#include <set>
#include <iostream>

using namespace mycad::topo;

SCENARIO( "002: Vertex Topology", "[topology][vertex]" )
{
    rc::prop("Each Vertex is given a unique ID",
        []()
        {
            const auto n = *rc::gen::inRange<unsigned int>(1, 1000);

            std::set<int> vals;

            Topology topo;
            for(unsigned int i=0; i <= n; i++)
            {
                auto [id] = topo.addFreeVertex();
                RC_ASSERT(vals.count(id) == (std::size_t) 0);
                vals.insert(id);
            }

            return true;
        },
        /* verbose= */ true
    );

    GIVEN("Two Vertices")
    {
        Topology topo;
        Topology unsafe_topo;

        VertexID v1 = topo.addFreeVertex();
        VertexID v2 = topo.addFreeVertex();

        VertexID unsafe_v1 = unsafe_topo.addFreeVertex();
        VertexID unsafe_v2 = unsafe_topo.addFreeVertex();

        REQUIRE(unsafe_topo.hasVertex(unsafe_v1));
        REQUIRE(unsafe_topo.hasVertex(unsafe_v2));

        WHEN("An Edge is created between them")
        {
            Topology orig = topo;
            auto eitherEdge = topo.makeEdge(v1, v2);
            EdgeID unsafe_edge = unsafe_topo.unsafe_makeEdge(unsafe_v1, unsafe_v2);

            REQUIRE(unsafe_topo.hasEdge(unsafe_edge));

            if (not eitherEdge.has_value())
            {
                std::cout << eitherEdge.error() << '\n';
            }

            THEN("A unique ID is returned for that Edge")
            {
                REQUIRE(eitherEdge.has_value());
            }

            EdgeID edge = eitherEdge.value();

            THEN("An adjacency exists between each Vertex and the new Edge")
            {
                // Either Edge ID from V₁
                REQUIRE(
                    topo.edgesAdjacentToVertex(v1).value()==
                    std::vector<EdgeID>{edge}
                );
                REQUIRE(
                    topo.edgesAdjacentToVertex(v2).value() ==
                    std::vector<EdgeID>{edge}
                );
                REQUIRE(
                    unsafe_topo.unsafe_edgesAdjacentToVertex(unsafe_v1) ==
                    std::vector<EdgeID>{unsafe_edge}
                );

                REQUIRE(
                    unsafe_topo.unsafe_edgesAdjacentToVertex(unsafe_v2) ==
                    std::vector<EdgeID>{unsafe_edge}
                );
            }

            THEN("Both Vertices are adjacent to the Edge")
            {
                REQUIRE(
                    topo.getEdgeVertices(edge).value() ==
                    std::pair<VertexID, VertexID>(v1, v2)
                );
                REQUIRE(
                    unsafe_topo.unsafe_getEdgeVertices(unsafe_edge) ==
                    std::pair<VertexID, VertexID>(unsafe_v1, unsafe_v2)
                );
            }

            THEN("Either Vertex can be used to find the other across the Edge")
            {
                REQUIRE(topo.oppositeVertex(v1, edge).value() == v2);
                REQUIRE(topo.oppositeVertex(v2, edge).value() == v1);
                REQUIRE( unsafe_topo.unsafe_oppositeVertex(unsafe_v1, unsafe_edge) == unsafe_v2);
                REQUIRE( unsafe_topo.unsafe_oppositeVertex(unsafe_v2, unsafe_edge) == unsafe_v1);
            }

            WHEN("The Edge is deleted")
            {
                CHECK(topo.deleteEdge(edge));

                THEN("The Topology reverts to the previous state")
                {
                    REQUIRE(orig.similar(topo));
                }
            }
        }

        WHEN("There's already an Edge between them")
        {
            auto eitherEdge = topo.makeEdge(v1, v2);

            THEN("We cannot create a second Edge")
            {
                CHECK_FALSE(topo.makeEdge(v1, v2).has_value());
                REQUIRE_FALSE(topo.makeEdge(v2, v1).has_value());
            }
        }
    }

}

SCENARIO("002: Edge Topology", "[topology][edge]")
{
    GIVEN("A topology with a single Edge")
    {
        Topology topo;
        VertexID v1 = topo.addFreeVertex();
        VertexID v2 = topo.addFreeVertex();
        EdgeID edge = topo.makeEdge(v1, v2).value();

        WHEN("A second Edge is added adjacent to v1")
        {
            VertexID v3 = topo.addFreeVertex();
            EdgeID edge2 = topo.makeEdge(v2, v3).value();
            THEN("v2 is adjacent to both edges")
            {
                auto eitherEdges = topo.edgesAdjacentToVertex(v2);
                REQUIRE(eitherEdges.has_value());
                REQUIRE(std::ranges::count(eitherEdges.value(), edge) == 1);
                REQUIRE(std::ranges::count(eitherEdges.value(), edge2) == 1);
            }

            /* WHEN("A Chain is made between both edges") */
            /* { */
            /*     REQUIRE(topo.makeChain(edge, edge2).has_value()); */

            /*     THEN("We can recover both Edges in order") */
            /*     { */
            /*         REQUIRE( */
            /*             topo.getChainEdges(v1, edge).value() == */
            /*             std::vector<EdgeID>{edge, edge2}); */
            /*     } */
            /* } */

            WHEN("The second Edge is deleted before making a Chain")
            {
                topo.deleteEdge(edge2);
                THEN("We get an error")
                {
                    REQUIRE_FALSE(topo.makeChain(edge, edge2).has_value());
                }
            }
        }

        WHEN("A second Edge is added with zero adjacencies to the first")
        {
            VertexID v3 = topo.addFreeVertex();
            VertexID v4 = topo.addFreeVertex();
            auto eitherEdge2 = topo.makeEdge(v3, v4);

            THEN("Trying to make a chain between them results in an error")
            {
                REQUIRE_FALSE(topo.makeChain(edge, eitherEdge2.value()).has_value());
            }
        }
    }
}
