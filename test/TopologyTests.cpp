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
        WHEN("An Edge is created between them") {
            THEN("An Edge with a unique ID is created") {
                Topology topo;
                int v1 = topo.addFreeVertex(),
                    v2 = topo.addFreeVertex();

                std::set<int> edgeIDs = topo.getEdgeIDs();
                auto eitherEdgeID = topo.makeEdge(v1, v2);

                CHECK(eitherEdgeID.has_value());
                CHECK(edgeIDs.count(eitherEdgeID.value()) == 0);
            }
        }
    }
}
