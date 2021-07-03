#include "Topology.h" // detail/Topology.h NOT mycad/Topology.h


tl::expected<void, std::string>
mycad::topo::detail::hasVertex(int v, std::unordered_set<int> vs)
{
    if (vs.count(v) == 0)
    {
        return tl::unexpected(
            std::string("Vertex with ID=") + 
            std::to_string(v) + " not found"
        );
    }

    return {};
}
