#include "mycad/render_helpers.h"

#include <algorithm>

Mesh::Mesh(Fragment const & frag)
{
    addFragment(frag);
}

void Mesh::addFragment(Fragment const & frag)
{
    // see if we already have this vertex
    auto const & [v0, v1, v2] = frag;
    auto addVertex = [this](Vertex const & vert)
    {
        auto ret = std::find(vertices.begin(), vertices.end(), vert);

        if (ret == vertices.end())
        {
            vertices.push_back(vert);
            indices.push_back(vertices.size() - 1);
        }
        else
        {
            // don't create a duplicate
            indices.push_back(ret - vertices.begin());
        }
    };

    std::ranges::for_each(std::vector<Vertex>{v0, v1, v2}, addVertex);
}

void Mesh::addFragments(std::vector<Fragment> const & frags)
{
    for (auto const & frag : frags)
    {
        addFragment(frag);
    }
}

uint64_t Mesh::sizeOfVertices() const
{
    return sizeof(vertices.at(0)) * vertices.size();
}

uint64_t Mesh::sizeOfIndices() const
{
    return sizeof(indices.at(0)) * indices.size();
}

auto Mesh::getVertices() const -> std::vector<Vertex> const &
{
    return vertices;
}

auto Mesh::getIndices() const -> std::vector<uint32_t> const &
{
    return indices;
}
