#include "mycad/detail/Topology.h"
#include "mycad/Topology.h"

#include <algorithm>
#include <iostream>

using namespace mycad;

auto detail::getCommonVertexID(EdgeID const eid1, EdgeID const eid2,
                               std::map<EdgeID, Edge> const &es) -> MaybeVertexID
{
    auto const edge1 = es.find(eid1);
    auto const edge2 = es.find(eid2);
    if (edge1 == es.end() || edge2 == es.end())
    {
        return std::nullopt;
    }

    auto const [v1, v2] = edge1->second.ends;
    auto const [v3, v4] = edge2->second.ends;

    if (v1 == v3)
    {
        return v1;
    }
    else if (v1 == v4)
    {
        return v1;
    }
    else if (v2 == v3)
    {
        return v2;
    }
    else if (v2 == v4)
    {
        return v2;
    }
    else
    {
        return std::nullopt;
    }
}

auto detail::linkedToEdge(EdgeID const e)
{
    return [e](detail::Link const l)
           {return l.parentEdge == e;};
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
                if (link.parentEdge != fromEdge)
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
