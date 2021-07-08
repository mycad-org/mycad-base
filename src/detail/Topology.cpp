#include "Topology.h" // detail/Topology.h NOT mycad/Topology.h

#include <algorithm>

using namespace mycad::topo;

tl::expected<void, std::string>
detail::hasVertex(const Vertex& v, const std::vector<Vertex>& vs)
{
    if (std::ranges::count(vs, v) == 0)
    {
        return tl::unexpected(
            std::string("Vertex with ID=") + 
            std::to_string(v.getIndex()) + " not found"
        );
    }

    return {};
}

tl::expected<void, std::string>
detail::hasEdge(const Edge& edge, const std::vector<Edge>& es)
{
    if (std::ranges::count(es, edge) == 1)
    {
        return {};
    }
    else
    {
        return tl::unexpected(
            std::string("Edge with ID=") +
            std::to_string(edge.getIndex()) + " not found");
    }
}

tl::expected<int, std::string>
detail::getCommonVertexID(
    const Edge& edge1, const Edge& edge2, const std::vector<Edge>& es)
{
    return
        detail::hasEdge(edge1, es)
        .and_then(std::bind(detail::hasEdge, edge2, std::cref(es)))
        .and_then([edge1, edge2, &es]() -> tl::expected<int, std::string>
        {
            auto [v1, v2] = es.at(edge1.getIndex()).getVertexIDs();
            auto [v3, v4] = es.at(edge2.getIndex()).getVertexIDs();

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
                    std::to_string(edge1.getIndex()) +
                    " and " + std::to_string(edge2.getIndex()) +
                    " do not appear to share a common Vertex");
            }
        });
}
