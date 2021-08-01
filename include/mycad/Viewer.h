#ifndef MYCAD_VIEWER_HEADER
#define MYCAD_VIEWER_HEADER

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace mycad
{
    class GLFW_Application
    {
        public:
            // initializes glfw (or tries to at least)
            GLFW_Application();
            // always calls glfwTerminate
            ~GLFW_Application();

            // true if glfwInit is true AND we were able to create a window
            explicit operator bool() const;

            // do with this what you please
            GLFWwindow* win = nullptr;

        private:
            void init();
            void makeWindow();
    };
} // namespace mycad
#endif
