#pragma once
#include <glm/glm.hpp>
#include <vector>

// Accumulates coloured line segments in world space and draws them in one call.
// Rebuilt each frame (wireframe edges, ground grid, MTV arrow), then uploaded.
class LineBatch {
public:
    void init();
    void clear();
    void addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);
    // Arrow from `from` to `to` with a small 3D arrowhead.
    void addArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
    void upload();         // push current segments to the GPU
    void draw() const;     // glDrawArrays(GL_LINES, ...)

private:
    struct LineVertex { glm::vec3 pos, color; };
    std::vector<LineVertex> verts;
    unsigned int vao = 0, vbo = 0;
    size_t       capacity = 0;   // current GPU buffer capacity (vertices)
};
