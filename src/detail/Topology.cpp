#include "mycad/detail/Topology.h"
#include "mycad/Topology.h"

#include <algorithm>

using namespace mycad::topo;

tl::expected<void, std::string>
detail::hasVertex(const VertexID& v, const std::map<VertexID, Vertex>& vs)
{
    if (vs.contains(v) == 0)
    {
        return tl::unexpected(
            std::string("Vertex with ID=") + 
            std::to_string(v.index) + " not found"
        );
    }

    return {};
}

tl::expected<void, std::string>
detail::hasEdge(const EdgeID& edge, const std::map<EdgeID, Edge>& es)
{
    if (es.count(edge) == 1)
    {
        return {};
    }
    else
    {
        return tl::unexpected(
            std::string("Edge with ID=") +
            std::to_string(edge.index) + " not found");
    }
}

tl::expected<int, std::string>
detail::getCommonVertexID(
    const EdgeID& edge1,
    const EdgeID& edge2,
    const std::map<EdgeID, Edge>& es)
{
    return
        hasEdge(edge1, es)
        .and_then(std::bind(hasEdge, edge2, std::cref(es)))
        .and_then([edge1, edge2, &es]() -> tl::expected<int, std::string>
        {
            auto [v1, v2] = es.at(edge1);
            auto [v3, v4] = es.at(edge2);

            if (v1 == v3)
            {
                return v1;
            }
            else if (v1 == v4)
            {
                return v2;
            }
            else if (v2 == v3)
            {
                return v2;
            }
            else if (v3 == v4)
            {
                return v4;
            }
            else
            {
                return tl::unexpected(
                    std::string("The two edges with IDs ") +
                    std::to_string(edge1.index) +
                    " and " + std::to_string(edge2.index) +
                    " do not appear to share a common Vertex");
            }
        });
}
