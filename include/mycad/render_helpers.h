#ifndef MYCAD_RENDER_HELPERS_HEADER
#define MYCAD_RENDER_HELPERS_HEADER

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include <vector>

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
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
