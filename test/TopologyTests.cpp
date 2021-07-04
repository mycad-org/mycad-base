#include "mycad/Topology.h"
#include "tl/expected.hpp"

#include <catch2/catch.hpp>
#include "rapidcheck.h"
#include "rapidcheck/catch.h"

#include <set>
#include <iostream>

using namespace mycad::topo;

SCENARIO( "002: Vertex Topology", "[topology][vertex]" ) {
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

    GIVEN("Two Vertices"){
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
                auto eEID_v1 = topo.edgesAdjacentToVertex(v1);
                auto eEID_v2 = topo.edgesAdjacentToVertex(v2);
                REQUIRE(eEID_v1.has_value());
                REQUIRE(eEID_v2.has_value());
                CHECK(eEID_v1.value() == std::unordered_set<int>{edgeID});
                REQUIRE(eEID_v2.value() == std::unordered_set<int>{edgeID});
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

            THEN("We cannot create a second Edge in the same direction")
            {
                CHECK_FALSE(topo.makeEdge(v1, v2).has_value());
            }

            THEN("We can still create an Edge in the reverse direction")
            {
                REQUIRE(topo.makeEdge(v2, v1).has_value());
            }
        }
    }
}
