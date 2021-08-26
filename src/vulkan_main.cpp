#include "mycad/vulkan_helpers.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

int main()
{
    // TODO: don't hard-code this
    Mesh squares{{
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    }};
    squares.addFragments({
        {
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
        },
        // add second square
        {
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        },
        {
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
        }
    });

    LineMesh lines{
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f}
    };
    lines.addSegment(
        {0.5f, -0.5f, 0.0f},
        {0.5f, 0.5f, 0.0f}
    );
    lines.addSegment(
        {0.5f, 0.5f, 0.0f},
        {-0.5f, 0.5f, 0.0f}
    );
    lines.addSegment(
        {-0.5f, 0.5f, 0.0f},
        {-0.5f, -0.5f, 0.0f}
    );
    lines.addSegment(
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f}
    );
    lines.addSegment(
        {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f}
    );
    lines.addSegment(
        {0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f}
    );
    lines.addSegment(
        {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f}
    );

    ApplicationData app;

    Renderer renderer(app.window, MAX_FRAMES_IN_FLIGHT);
    renderer.addMesh(squares);
    renderer.addLineMesh(lines);

    int currentFrame = 0;
    /* int i = 0; */
    while(!glfwWindowShouldClose(app.window))
    {
        renderer.draw(currentFrame);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        glfwPollEvents();
        /* i++; */
    }
}
