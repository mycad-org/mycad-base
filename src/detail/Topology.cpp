#include "mycad/detail/Topology.h"
#include "mycad/Topology.h"

#include <algorithm>
#include <iostream>

using namespace mycad::topo;

auto detail::getCommonVertexID(EdgeID const edge1, EdgeID const edge2,
                               std::map<EdgeID, Edge> const &es) -> VertexID
{
    auto pair1 = es.find(edge1);
    auto pair2 = es.find(edge2);
    if (pair1 == es.end() || pair2 == es.end())
    {
        return {-1};
    }

    auto [v1, v2] = pair1->second;
    auto [v3, v4] = pair2->second;

    if (v1 == v3)
    {
        return {v1};
    }
    else if (v1 == v4)
    {
        return {v1};
    }
    else if (v2 == v3)
    {
        return {v2};
    }
    else if (v2 == v4)
    {
        return {v2};
    }
    else
    {
        return {-1};
    }
}

auto detail::linkedToEdge(EdgeID const e)
{
    return [e](detail::Link const l)
           {return l.parentEdgeIndex == e.index;};
}
