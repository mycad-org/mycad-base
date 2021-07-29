#include "mycad/Topology.h"

#include <catch2/catch.hpp>
#include "rapidcheck.h"
#include "rapidcheck/catch.h"

#include <algorithm>
#include <set>
#include <iostream>

SCENARIO( "002: Vertex Topology", "[topology][vertex]" )
{
    rc::prop("Each Vertex is given a unique ID",
        []()
        {
            auto const n = *rc::gen::inRange<unsigned int>(1, 1000);

            std::set<int> vals;

            mycad::Topology topo;
            for(unsigned int i=0; i <= n; i++)
            {
                mycad::VertexID id = topo.addFreeVertex();
                RC_ASSERT(vals.count(id) == (std::size_t) 0);
                vals.insert(id);
            }

            return true;
        },
        /* verbose= */ true
    );

    GIVEN("Two Vertices")
    {
        mycad::Topology topo;

        mycad::VertexID v1 = topo.addFreeVertex();
        mycad::VertexID v2 = topo.addFreeVertex();

        WHEN("An Edge is created between them")
        {
            mycad::Topology orig = topo;
            mycad::MaybeEdgeID edge   = topo.makeEdge(v1, v2);

            THEN("The returned Edge is valid within the Topology")
            {
                REQUIRE(edge.has_value());
            }

            THEN("An adjacency exists between each Vertex and the new Edge")
            {
                REQUIRE( *topo.edgesAdjacentToVertex(v1) == mycad::EdgeIDs{*edge});
                REQUIRE( *topo.edgesAdjacentToVertex(v2) == mycad::EdgeIDs{*edge});
            }

            THEN("Both Vertices are adjacent to the Edge")
            {
                REQUIRE(topo.getEdgeVertices(edge).value() == mycad::VertexIDPair{v1, v2});
            }

            THEN("Either Vertex can be used to find the other across the Edge")
            {
                REQUIRE(topo.oppositeVertex(v1, *edge) == v2);
                REQUIRE(topo.oppositeVertex(v2, *edge) == v1);
            }

            WHEN("The Edge is deleted")
            {
                CHECK(topo.deleteEdge(*edge));

                THEN("The Topology reverts to the previous state")
                {
                    REQUIRE(orig.similar(topo));
                }
            }
        }

        WHEN("There's already an Edge between them")
        {
            topo.makeEdge(v1, v2);

            THEN("We cannot create a second Edge")
            {
                mycad::MaybeEdgeID sameOrder = topo.makeEdge(v1, v2);
                mycad::MaybeEdgeID revOrder  = topo.makeEdge(v2, v1);
                CHECK_FALSE(sameOrder.has_value());
                REQUIRE_FALSE(revOrder.has_value());
            }
        }
    }

}

SCENARIO("003: Edge Topology", "[topology][edge]")
{
    GIVEN("A topology with a single Edge")
    {
        mycad::Topology topo;
        mycad::VertexID v1 = topo.addFreeVertex();
        mycad::VertexID v2 = topo.addFreeVertex();
        mycad::EdgeID edge = topo.makeEdge(v1, v2).value();

        WHEN("A second Edge is added adjacent to v1")
        {
            mycad::VertexID v3 = topo.addFreeVertex();
            mycad::EdgeID edge2 = topo.makeEdge(v2, v3).value();
            THEN("v2 is adjacent to both edges")
            {
                auto edges = topo.edgesAdjacentToVertex(v2);
                REQUIRE(std::ranges::count(*edges, edge) == 1);
                REQUIRE(std::ranges::count(*edges, edge2) == 1);
            }

            WHEN("The two Edges are joined")
            {
                auto chain = topo.joinEdges(edge, edge2);

                THEN("We can recover both Edges in order using the returned Chain")
                {
                    REQUIRE(topo.getChainEdges(*chain) == mycad::EdgeIDs{edge, edge2});
                }
            }

            WHEN("The second Edge is deleted before trying to join them")
            {
                topo.deleteEdge(edge2);
                THEN("We get an error")
                {
                    mycad::MaybeChain c = topo.joinEdges(edge, edge2);
                    REQUIRE_FALSE(topo.hasChain(*c));
                }
            }
        }

        WHEN("A second Edge is added with zero adjacencies to the first")
        {
            mycad::VertexID v3 = topo.addFreeVertex();
            mycad::VertexID v4 = topo.addFreeVertex();
            auto edge2 = topo.makeEdge(v3, v4);

            THEN("Trying to join them results in an error")
            {
                mycad::MaybeChain c = topo.joinEdges(edge, edge2);
                REQUIRE_FALSE(topo.hasChain(*c));
            }
        }
    }
}

SCENARIO("004: Chain Topology", "[topology][chain]")
{
    GIVEN("A series of three Edges")
    {
        mycad::Topology topo;
        auto v1 = topo.addFreeVertex();
        auto v2 = topo.addFreeVertex();
        auto v3 = topo.addFreeVertex();
        auto v4 = topo.addFreeVertex();
        auto e1 = topo.makeEdge(v1, v2).value();
        auto e2 = topo.makeEdge(v2, v3).value();
        auto e3 = topo.makeEdge(v3, v4).value();

        WHEN("The first two are connected")
        {
            mycad::Chain chain = topo.joinEdges(e1, e2).value();

            THEN("The third is not part of the chain")
            {
                auto edges = topo.getChainEdges(chain);

                REQUIRE(std::ranges::count(*edges, e3) == 0);
            }

            WHEN("The second two are connected")
            {
                mycad::Chain c2 = topo.joinEdges(e2, e3).value();

                THEN("The third is now part of the original chain")
                {
                    auto edges = topo.getChainEdges(chain);

                    REQUIRE(std::ranges::count(*edges, e3) == 1);
                }

                THEN("The third can be retrieved using the second returned chain")
                {
                    auto edges = topo.getChainEdges(c2);

                    REQUIRE(std::ranges::count(*edges, e3) == 1);
                }

                THEN("The second chain returns a sub-set of the first chain")
                {
                    auto edges1 = topo.getChainEdges(chain);
                    auto edges2 = topo.getChainEdges(c2);

                    // both must be sorted for ranges::includes to work
                    std::ranges::sort(*edges1);
                    std::ranges::sort(*edges2);

                    REQUIRE(edges2.value().size() < edges1.value().size());
                    REQUIRE(std::ranges::includes(*edges1, *edges2));
                }
            }

            WHEN("The first chain is extended with the third Edge")
            {
                topo.extendChain(chain, e3);

                THEN("All three edges can be retrieved")
                {
                    auto edges = topo.getChainEdges(chain);

                    REQUIRE(*edges == std::vector<mycad::EdgeID>{e1, e2, e3});
                }
            }
        }
    }

    GIVEN("Three Edge with one common Vertex")
    {
        mycad::Topology topo;
        auto v0 = topo.addFreeVertex();
        auto v1 = topo.addFreeVertex();
        auto v2 = topo.addFreeVertex();
        auto v3 = topo.addFreeVertex();
        auto e0 = topo.makeEdge(v1, v0).value();
        auto e1 = topo.makeEdge(v2, v0).value();
        auto e2 = topo.makeEdge(v3, v0).value();

        WHEN("Edge e0 is joined to e1")
        {
            mycad::Chain c = topo.joinEdges(e0, e1).value();

            REQUIRE(topo.hasChain(c));

            THEN("Edge e0 cannot be used again as a FROM Edge in joinEdges")
            {
                mycad::MaybeChain c2 = topo.joinEdges(e0, e2);

                REQUIRE_FALSE(topo.hasChain(*c2));
            }

            THEN("Edge e0 CAN be used as a TO Edge in joinEdges")
            {
                mycad::MaybeChain c2   = topo.joinEdges(e2, e0);
                auto edges = topo.getChainEdges(*c2);

                REQUIRE(*edges == std::vector<mycad::EdgeID>{e2, e0});
            }

            THEN("Edge e1 cannot be used as a TO edge in joinEdges")
            {
                mycad::MaybeChain c2 = topo.joinEdges(e2, e1);

                REQUIRE_FALSE(topo.hasChain(*c2));
            }
        }
    }

    GIVEN("Three Edge that form a closed loop")
    {
        mycad::Topology topo;
        auto v0 = topo.addFreeVertex();
        auto v1 = topo.addFreeVertex();
        auto v2 = topo.addFreeVertex();
        auto v3 = topo.addFreeVertex();
        auto e0 = topo.makeEdge(v1, v0).value();
        auto e1 = topo.makeEdge(v2, v0).value();
        auto e2 = topo.makeEdge(v3, v0).value();

        WHEN("They are joined into a Chain")
        {
            mycad::Chain c = topo.joinEdges(e0, e1).value();
            topo.joinEdges(e1, e2);
            topo.joinEdges(e2, e0);

            REQUIRE(topo.hasChain(c));

            THEN("We can retrieve a list of Edges in the loop")
            {
                mycad::MaybeEdgeIDs mIDs = topo.getChainEdges(c);

                REQUIRE(mIDs.has_value());
                REQUIRE(*mIDs == mycad::EdgeIDs{e0, e1, e2});
            }
        }
    }
}
