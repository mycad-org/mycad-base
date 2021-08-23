#ifndef MYCAD_RENDER_HELPERS_HEADER
#define MYCAD_RENDER_HELPERS_HEADER

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include <array>
#include <vector>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(Vertex const &) const = default;
};

// Order matters: v0 → v1 → v2 _will_ be visible if they are oriented
// counter-clockwise to each other, and will _not_ be visible otherwise.
struct Fragment
{
    Vertex v0;
    Vertex v1;
    Vertex v2;
};

class Mesh
{
    public:
        // A surface must have at one fragment
        explicit Mesh(Fragment const & frag);

        // More resolution can be accomplished by increasing the frag count
        void addFragment(Fragment const & frag);
        void addFragments(std::vector<Fragment> const & frags);

        // A "thick line" can be rendered using a particular shader
        void addLine(Vertex const & v0, Vertex const & v1);

        uint64_t sizeOfVertices() const;
        uint64_t sizeOfIndices() const;

        auto getVertices() const -> std::vector<Vertex> const &;
        auto getAllIndices() const -> std::vector<uint32_t>;
        auto getVertexIndices() const -> std::vector<uint32_t> const &;
        auto getLineIndices() const -> std::vector<uint32_t> const &;
        uint64_t lineIndicesOffset() const;

    private:
        // Returns the index to indices of the added vertex - this function will
        // avoid adding duplicate vertices
        std::size_t addVertex(Vertex const & vertex);

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> lineIndices;
};

// alignas added explicitly to remind you in the future in case you have
// unaligned member variables
struct MVPBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

#endif
