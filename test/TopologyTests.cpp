#include "mycad/Topology.h"
#include "tl/expected.hpp"

#include <catch2/catch.hpp>
#include "rapidcheck.h"
#include "rapidcheck/catch.h"

using namespace mycad::topo;

SCENARIO( "002: Vertex Topology", "[topology][vertex]" ) {
    rc::prop("RemoveVertex restores the state previous to AddVertex",
        [](const Topology& t) {
            EitherTopologyOrString eTopo= t.addVertex()
                                            .and_then(&Topology::removeVertex);;
            RC_ASSERT(eTopo.has_value());
            RC_ASSERT(eTopo.value() == t);
        }
    );
}
