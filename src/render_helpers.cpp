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

LineMesh::LineMesh(glm::vec3 const & v0, glm::vec3 const & v1)
{
    addSegment(v0, v1);
}

void LineMesh::addSegment(glm::vec3 const & v0, glm::vec3 const & v1)
{
    glm::vec3 dir = glm::normalize(v1 - v0);

    addVertex({v0, dir, 1});
    addVertex({v0, dir, -1});
    addVertex({v1, -dir, 1});
    addVertex({v0, dir,  1});
    addVertex({v1, -dir, 1});
    addVertex({v1, -dir, -1});
}

uint64_t LineMesh::sizeOfVertices() const
{
    return sizeof(vertices.at(0)) * vertices.size();
}

uint64_t LineMesh::sizeOfIndices() const
{
    return sizeof(indices.at(0)) * indices.size();
}

auto LineMesh::getVertices() const -> std::vector<LineVertex> const &
{
    return vertices;
}

auto LineMesh::getIndices() const -> std::vector<uint32_t> const &
{
    return indices;
}

std::size_t LineMesh::addVertex(LineVertex const & vertex)
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
