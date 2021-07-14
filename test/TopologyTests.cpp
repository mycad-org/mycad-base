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
            auto const n = *rc::gen::inRange<unsigned int>(1, 1000);

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

        VertexID v1 = topo.addFreeVertex();
        VertexID v2 = topo.addFreeVertex();

        WHEN("An Edge is created between them")
        {
            Topology orig = topo;
            EdgeID edge   = topo.makeEdge(v1, v2);

            THEN("The returned Edge is valid within the Topology")
            {
                REQUIRE(topo.hasEdge(edge));
            }

            THEN("An adjacency exists between each Vertex and the new Edge")
            {
                // Either Edge ID from V₁
                REQUIRE( topo.edgesAdjacentToVertex(v1) == EdgeIDs{edge});
                REQUIRE( topo.edgesAdjacentToVertex(v2) == EdgeIDs{edge});
            }

            THEN("Both Vertices are adjacent to the Edge")
            {
                REQUIRE(topo.getEdgeVertices(edge) == VertexIDPair{v1, v2});
            }

            THEN("Either Vertex can be used to find the other across the Edge")
            {
                REQUIRE(topo.oppositeVertex(v1, edge) == v2);
                REQUIRE(topo.oppositeVertex(v2, edge) == v1);
            }

/*             WHEN("The Edge is deleted") */
/*             { */
/*                 CHECK(topo.deleteEdge(edge)); */

/*                 THEN("The Topology reverts to the previous state") */
/*                 { */
/*                     REQUIRE(orig.similar(topo)); */
/*                 } */
/*             } */
        }

        WHEN("There's already an Edge between them")
        {
            topo.makeEdge(v1, v2);

            THEN("We cannot create a second Edge")
            {
                EdgeID sameOrder = topo.makeEdge(v1, v2);
                EdgeID revOrder  = topo.makeEdge(v2, v1);
                CHECK_FALSE(topo.hasEdge(sameOrder));
                REQUIRE_FALSE(topo.hasEdge(revOrder));
            }
        }
    }

}

/* SCENARIO("003: Edge Topology", "[topology][edge]") */
/* { */
/*     GIVEN("A topology with a single Edge") */
/*     { */
/*         Topology topo; */
/*         VertexID v1 = topo.addFreeVertex(); */
/*         VertexID v2 = topo.addFreeVertex(); */
/*         EdgeID edge = topo.makeEdge(v1, v2).value(); */

/*         WHEN("A second Edge is added adjacent to v1") */
/*         { */
/*             VertexID v3 = topo.addFreeVertex(); */
/*             EdgeID edge2 = topo.makeEdge(v2, v3).value(); */
/*             THEN("v2 is adjacent to both edges") */
/*             { */
/*                 auto eitherEdges = topo.edgesAdjacentToVertex(v2); */
/*                 REQUIRE(eitherEdges.has_value()); */
/*                 REQUIRE(std::ranges::count(eitherEdges.value(), edge) == 1); */
/*                 REQUIRE(std::ranges::count(eitherEdges.value(), edge2) == 1); */
/*             } */

/*             WHEN("A Chain is made between both edges") */
/*             { */
/*                 auto eitherChain = topo.makeChain(edge, edge2); */
/*                 REQUIRE(eitherChain.has_value()); */

/*                 THEN("We can recover both Edges in order using the returned Chain") */
/*                 { */
/*                     Chain chain = eitherChain.value(); */
/*                     REQUIRE( */
/*                         topo.getChainEdges(chain).value() == */
/*                         std::vector<EdgeID>{edge, edge2}); */
/*                 } */
/*             } */

/*             WHEN("The second Edge is deleted before making a Chain") */
/*             { */
/*                 topo.deleteEdge(edge2); */
/*                 THEN("We get an error") */
/*                 { */
/*                     REQUIRE_FALSE(topo.makeChain(edge, edge2).has_value()); */
/*                 } */
/*             } */
/*         } */

/*         WHEN("A second Edge is added with zero adjacencies to the first") */
/*         { */
/*             VertexID v3 = topo.addFreeVertex(); */
/*             VertexID v4 = topo.addFreeVertex(); */
/*             auto eitherEdge2 = topo.makeEdge(v3, v4); */

/*             THEN("Trying to make a chain between them results in an error") */
/*             { */
/*                 REQUIRE_FALSE(topo.makeChain(edge, eitherEdge2.value()).has_value()); */
/*             } */
/*         } */
/*     } */
/* } */

/* SCENARIO("004: Chain Topology", "[topology][chain]") */
/* { */
/*     GIVEN("A series of three Edges") */
/*     { */
/*         Topology topo; */
/*         auto v1 = topo.addFreeVertex(); */
/*         auto v2 = topo.addFreeVertex(); */
/*         auto v3 = topo.addFreeVertex(); */
/*         auto v4 = topo.addFreeVertex(); */
/*         // unsafe_makeEdge here is "ok" since we **know** that there four */
/*         // vertices exist in the topology. generally though, avoid this. */
/*         auto e1 = topo.unsafe_makeEdge(v1, v2); */
/*         auto e2 = topo.unsafe_makeEdge(v2, v3); */
/*         auto e3 = topo.unsafe_makeEdge(v3, v4); */

/*         WHEN("The first two are connected") */
/*         { */
/*             auto eitherChain = topo.makeChain(e1, e2); */
/*             REQUIRE(eitherChain.has_value()); */
/*             Chain chain = *eitherChain; */

/*             THEN("The third is not part of the chain") */
/*             { */
/*                 auto eitherEdges = topo.getChainEdges({v1, e1}); */

/*                 REQUIRE(eitherEdges.has_value()); */
/*                 REQUIRE(std::ranges::count(eitherEdges.value(), e3) == 0); */
/*             } */

/*             WHEN("The second two are connected") */
/*             { */
/*                 topo.makeChain(e2, e3); */

/*                 THEN("The third is still not part of the original chain") */
/*                 { */
/*                     auto eitherEdges = topo.getChainEdges(chain); */

/*                     REQUIRE(eitherEdges.has_value()); */
/*                     REQUIRE(std::ranges::count(eitherEdges.value(), e3) == 0); */
/*                 } */

/*                 THEN("The third can be retrieved using v2 and e2") */
/*                 { */
/*                     auto eitherEdges = topo.getChainEdges({v2, e2}); */

/*                     REQUIRE(eitherEdges.has_value()); */
/*                     REQUIRE(eitherEdges.value() == */
/*                             std::vector<EdgeID>{e2, e3}); */
/*                 } */
/*             } */
/*         } */
/*     } */
/* } */
