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
                int id = topo.addFreeVertex();
                RC_ASSERT(vals.count(id) == (std::size_t) 0);
                vals.insert(id);
            }

            return true;
        }
    );

    GIVEN("Two Vertices")
    {
        Topology topo;
        int v1 = topo.addFreeVertex(),
            v2 = topo.addFreeVertex();

        WHEN("An Edge is created between them")
        {
            Topology orig = topo;
            auto eitherEdgeID = topo.makeEdge(v1, v2);

            THEN("A unique ID is returned for that Edge")
            {
                REQUIRE(eitherEdgeID.has_value());
            }

            int edgeID = eitherEdgeID.value();

            THEN("An adjacency exists between each Vertex and the new Edge")
            {
                // Either Edge ID from V₁
                REQUIRE(
                    topo.edgesAdjacentToVertex(v1).value()==
                    std::unordered_set<int>{edgeID}
                );
                REQUIRE(
                    topo.edgesAdjacentToVertex(v2).value() ==
                    std::unordered_set<int>{edgeID}
                );
            }

            THEN("Both Vertices are adjacent to the Edge")
            {
                REQUIRE(
                    topo.getEdgeVertices(edgeID).value() ==
                    std::pair<int, int>(v1, v2)
                );
            }

            THEN("Either Vertex can be used to find the other across the Edge")
            {
                REQUIRE(topo.oppositeVertex(v1, edgeID).value() == v2);
                REQUIRE(topo.oppositeVertex(v2, edgeID).value() == v1);
            }

            WHEN("The Edge is deleted")
            {
                CHECK(topo.deleteEdge(edgeID));

                THEN("The Topology reverts to the previous state")
                {
                    REQUIRE(orig.similar(topo));
                }
            }
        }

        WHEN("There's already an Edge between them")
        {
            auto eitherEdgeID = topo.makeEdge(v1, v2);

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
        int v1 = topo.addFreeVertex();
        int v2 = topo.addFreeVertex();
        int edge = topo.makeEdge(v1, v2).value();

        WHEN("A second Edge is added adjacent to v1")
        {
            int v3 = topo.addFreeVertex();
            int edge2 = topo.makeEdge(v2, v3).value();
            THEN("v2 is adjacent to both edges")
            {
                auto eitherEdges = topo.edgesAdjacentToVertex(v2);
                REQUIRE(eitherEdges.has_value());
                REQUIRE(eitherEdges.value().contains(edge));
                REQUIRE(eitherEdges.value().contains(edge2));
            }

            WHEN("A Chain is made between both edges")
            {
                REQUIRE(topo.makeChain(edge, edge2).has_value());

                THEN("We can recover both Edges in order")
                {
                    REQUIRE(topo.getChainEdges(v1, edge).value() == std::list<int>{edge, edge2});
                }
            }

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
            int v3 = topo.addFreeVertex();
            int v4 = topo.addFreeVertex();
            int edge2 = topo.makeEdge(v3, v4).value();

            THEN("Trying to make a chain between them results in an error")
            {
                REQUIRE_FALSE(topo.makeChain(edge, edge2).has_value());
            }
        }
    }
}
