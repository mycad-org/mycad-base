#include "Topology.h" // detail/Topology.h NOT mycad/Topology.h

#include <algorithm>

tl::expected<void, std::string>
mycad::topo::detail::hasVertex(int v, std::vector<int> vs)
{
    if (std::ranges::count(vs, v) == 0)
    {
        return tl::unexpected(
            std::string("Vertex with ID=") + 
            std::to_string(v) + " not found"
        );
    }

    return {};
}

tl::expected<void, std::string>
mycad::topo::detail::hasEdge(int edge, const std::map<int, std::unique_ptr<Edge>>& es)
{
    if (not es.contains(edge))
    {
        return tl::unexpected(
            std::string("Edge with ID=") +
            std::to_string(edge) + " not found"
        );
    }

    return {};
}

tl::expected<int, std::string>
mycad::topo::detail::getCommonVertex(
    int edge1, int edge2, const std::map<int, std::unique_ptr<Edge>>& es)
{
    return
        hasEdge(edge1, es)
        .and_then(std::bind(hasEdge, edge2, std::cref(es)))
        .and_then([edge1, edge2, &es]() -> tl::expected<int, std::string>
        {
            auto [v1, v2] = *(es.at(edge1));
            auto [v3, v4] = *(es.at(edge2));

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
                    std::string("The two edges with IDs ") + std::to_string(edge1) +
                    " and " + std::to_string(edge2) +
                    " do not appear to share a common Vertex");
            }
        });
}
