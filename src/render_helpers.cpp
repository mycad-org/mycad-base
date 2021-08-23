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

    std::ranges::for_each(
        std::vector<Vertex>{v0, v1, v2},
        [this](Vertex const & vert){addVertex(vert);}
    );
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
    return lineIndicesOffset() + sizeof(lineIndices.at(0)) * lineIndices.size();
}

auto Mesh::getVertices() const -> std::vector<Vertex> const &
{
    return vertices;
}

auto Mesh::getAllIndices() const -> std::vector<uint32_t>
{
    std::vector<uint32_t> copyOfIndices = indices;

    copyOfIndices.insert(copyOfIndices.end(), lineIndices.begin(), lineIndices.end());

    return copyOfIndices;
}

auto Mesh::getVertexIndices() const -> std::vector<uint32_t> const &
{
    return indices;
}

auto Mesh::getLineIndices() const -> std::vector<uint32_t> const &
{
    return lineIndices;
}

uint64_t Mesh::lineIndicesOffset() const
{
    return sizeof(indices.at(0)) * indices.size();
}

void Mesh::addLine(Vertex const & v0, Vertex const & v1)
{
    auto i1 = addVertex(v0);
    auto i2 = addVertex(v1);

    lineIndices.insert(lineIndices.end(), 2, i1);
    lineIndices.insert(lineIndices.end(), 2, i2);
}

std::size_t Mesh::addVertex(Vertex const & vertex)
{
    auto ret = std::find(vertices.begin(), vertices.end(), vertex);

    std::size_t index = 0;
    if (ret == vertices.end())
    {
        vertices.push_back(vertex);
        index = vertices.size() - 1;
    }
    else
    {
        // don't create a duplicate
        index = ret - vertices.begin();
    }

    indices.push_back(index);
    return index;
}
