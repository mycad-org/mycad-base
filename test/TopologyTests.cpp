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
        []() {
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

    GIVEN("Two Vertices") {
        Topology topo;
        int v1 = topo.addFreeVertex(),
            v2 = topo.addFreeVertex();

        WHEN("An Edge is created between them") {
            auto eitherEdgeID = topo.makeEdge(v1, v2);

            THEN("A unique ID is returned for that Edge") {
                REQUIRE(eitherEdgeID.has_value());
            }
        }

        WHEN("There's already an Edge between them") {
            auto eitherEdgeID = topo.makeEdge(v1, v2);

            THEN("We cannot create a second Edge in the same direction") {
                CHECK_FALSE(topo.makeEdge(v1, v2).has_value());
            }

            THEN("We can still create an Edge in the reverse direction") {
                REQUIRE(topo.makeEdge(v2, v1).has_value());
            }
        }
    }
}
