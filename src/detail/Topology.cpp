#include "mycad/detail/Topology.h"
#include "mycad/Topology.h"

#include <algorithm>
#include <iostream>

using namespace mycad;

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
           {return l.parentEdgeIndex == e;};
}

auto detail::isFromEdge
    (EdgeID const fromEdge,
     std::vector<detail::Link> const &commonVertexLinks) -> bool
{
    return
        std::ranges::any_of(
            commonVertexLinks,
            [fromEdge](detail::Link const &link)
            {
                if (link.parentEdgeIndex != fromEdge)
                {
                    return false;
                }

                return link.next.has_value();
            });
}

auto detail::isToEdge
    (EdgeID const toEdge,
     std::vector<detail::Link> const &commonVertexLinks) -> bool
{
    return
        std::ranges::any_of(
            commonVertexLinks,
            [toEdge](detail::Link const &link)
            {
                if (!link.next)
                {
                    return false;
                }

                return link.next->second == toEdge;
            });
}
