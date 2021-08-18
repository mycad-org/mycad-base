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

class Surface
{
    public:
        // A surface must have at one fragment
        explicit Surface(Fragment const & frag);

        // More resolution can be accomplished by increasing the frag count
        void addData(std::vector<Fragment> const & frags);

        uint64_t sizeOfVertices() const;
        uint64_t sizeOfIndices() const;

        auto getVertices() const -> std::vector<Vertex> const &;
        auto getIndices() const -> std::vector<uint32_t> const &;

    private:
        void addFragment(Fragment const & frag);

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
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
