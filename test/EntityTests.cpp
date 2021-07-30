#include "mycad/Entity.h"
#include "Arbitrary.h"

#include <catch2/catch.hpp>
#include "rapidcheck/catch.h"

SCENARIO( "004: Vertex Entity", "[entity][vertex]" )
{
    rc::prop("A Point can be recovered using a Vertex",
        [](mycad::Point const &point)
        {
            mycad::Entity entity;
            auto vertex = entity.addVertex(point);
            RC_ASSERT(entity.getPoint(vertex) == point);
        },
        /* verbose= */ true
    );

    rc::prop("A Line can be recovered using an Edge",
        [](mycad::Point const &p1)
        {
            auto p2 = *rc::gen::distinctFrom(p1);

            mycad::Entity entity;
            auto v1 = entity.addVertex(p1);
            auto v2 = entity.addVertex(p2);
            mycad::MaybeLine line = makeLine(p1, p2);

            auto edge = entity.addEdge(v1, v2);

            RC_ASSERT(line.has_value());
            RC_ASSERT(edge.has_value());
            mycad::Line l1 = entity.getLine(*edge).value();
            mycad::Line l2 = line.value();
            RC_ASSERT(l1 == l2);
        },
        /* verbose= */ true
    );
}
