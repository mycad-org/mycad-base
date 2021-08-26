#include "mycad/vulkan_helpers.h"
#include "mycad/Entity.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

int main()
{
    mycad::Entity ent;
    auto v0 = ent.addVertex({0, 0, 0});
    auto v1 = ent.addVertex({0.5, 0.5, 0});
    auto v2 = ent.addVertex({0.0, 0.5, 0});
    auto v3 = ent.addVertex({-0.5, -0.5, 0});

    ent.addEdge(v0, v1);
    ent.addEdge(v1, v2);
    ent.addEdge(v2, v3);
    ent.addEdge(v3, v1);

    auto edges = ent.getEdges();
    auto getStartFinish = [](mycad::Line const & line) -> std::pair<glm::vec3, glm::vec3>
    {
        auto p0 = line.atU(0);
        auto p1 = line.atU(1);

        return {{p0.x, p0.y, p0.z}, {p1.x, p1.y, p1.z}};
    };

    auto [left, right] = getStartFinish(edges.front());

    LineMesh lines{left, right};

    for(auto const & edge : edges)
    {
        if (edge == edges.front())
        {
            continue;
        }

        auto [left, right] = getStartFinish(edge);
        lines.addSegment(left, right);
    }

    ApplicationData app;

    Renderer renderer(app.window, MAX_FRAMES_IN_FLIGHT);
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
