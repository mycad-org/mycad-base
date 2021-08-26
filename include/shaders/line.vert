#version 450 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inDir;
layout (location = 2) in float up;

layout(binding = 0) uniform MVPBuferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(binding = 2) uniform LineUniform
{
    float aspect;
    float thickness;
} line;

void main()
{
    // This is the fully transformed "clip space" representation of our position
    vec4 clipPos = mvp.proj * mvp.view * mvp.model * vec4(inPos, 1.0);
    vec4 clipDir = mvp.proj * mvp.view * mvp.model * vec4(inDir, 1.0);

    // Dividing by "w" converts to Normalized Device Coordinates, NDC, [-1..1]
    // Must also correct for aspect ratio, which is screen.x / screen.y
    vec2 aspectVec = vec2(line.aspect, 1.0);
    vec2 ndcPos = clipPos.xy / clipPos.w; // * aspectVec;
    vec2 ndcDir = clipDir.xy / clipDir.w; // * aspectVec;
    ndcPos.x /= line.aspect;
    ndcDir.x /= line.aspect;

    // This vector is the direction of the line in NDC
    vec2 dir = normalize(ndcDir - ndcPos);

    // since it's 2-d, we can easily calculate the normal
    vec2 normal = vec2(-dir.y, dir.x);

    // The normal is the direction we'll "extrude" the line, by half the thickness
    normal *= line.thickness;
    // Not sure about these maths...
    normal.x *= line.aspect;

    vec4 delta = vec4(0.0f);
    if (up < 0)
    {
        delta = vec4(-1 * normal, 0.0, 1.0);
    }
    else
    {
        delta = vec4(normal, 0.0, 1.0);
    }

    // Finally, transform our point.
    gl_Position = clipPos + delta;
}
