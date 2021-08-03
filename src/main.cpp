// This is only a minimal viewer for mycad data structures. It is not (yet)
// intended to be a full CAD package

#include "mycad/Viewer.h"

#include <iostream>

const char* VSHADER_SOURCE = R"shd(
#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)shd";

const char* FSHADER_SOURCE = R"shd(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)shd";

int main()
{
    // Simple RAII, glfwTerminate will _always_ be called
    if(auto app = mycad::GLFW_Application())
    {
        // The vertices to draw
        float vertices[] = {
             0.5f,  0.5f, 0.0f,  // top right
             0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f,  0.5f, 0.0f   // top left
        };
        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        // Set up the Vertex Array Object to store the vbo, vertex attribute
        // array, and the ebo
        unsigned int vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // the memory on the gpu that holds the vertex info
        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // How to interpret the memory
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        // which indices to draw, and in which order
        unsigned int ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // compile shader program
        unsigned int vshader, fshader;
        vshader = glCreateShader(GL_VERTEX_SHADER);
        fshader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vshader, 1, &VSHADER_SOURCE, nullptr);
        glShaderSource(fshader, 1, &FSHADER_SOURCE, nullptr);

        glCompileShader(vshader);
        int success;
        char infoLog[512];
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(vshader, 512, NULL, infoLog);
            std::cerr << "ERROR: Vertex shader compilation failed: " << infoLog << std::endl;
        }

        glCompileShader(fshader);
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(fshader, 512, NULL, infoLog);
            std::cerr << "ERROR: Fragment shader compilation failed: " << infoLog << std::endl;
        }

        unsigned int shader;
        shader = glCreateProgram();
        glAttachShader(shader, vshader);
        glAttachShader(shader, fshader);
        glLinkProgram(shader);

        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR: Could not link Shader: " << infoLog << std::endl;
        }

        /* glDeleteShader(vshader); */
        /* glDeleteShader(fshader); */

        while(!glfwWindowShouldClose(app.win))
        {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(shader);
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glfwSwapBuffers(app.win);
            glfwPollEvents();
        }
    }
}
