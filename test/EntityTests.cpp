#include "mycad/Entity.h"
#include "Arbitrary.h"

#include <catch2/catch.hpp>
#include "rapidcheck/catch.h"

using namespace mycad;

SCENARIO( "004: Vertex Entity", "[entity][vertex]" )
{
    rc::prop("A Point can be recovered using a Vertex",
        [](Point const &point)
        {
            Entity entity;
            auto vertex = entity.addVertex(point);
            RC_ASSERT(entity.getPoint(vertex) == point);
        },
        /* verbose= */ true
    );
}
