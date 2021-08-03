#ifndef MYCAD_GL_RENDERER_HEADER
#define MYCAD_GL_RENDERER_HEADER

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace mycad
{
    class Renderer
    {
        public:
            explicit Renderer();
            void draw(GLFWwindow* win);

        private:
            unsigned int shader;
            unsigned int vao;
    };
}

#endif
