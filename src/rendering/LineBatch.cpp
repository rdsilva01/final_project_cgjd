#include "LineBatch.h"
#include <glad/glad.h>
#include <cmath>

void LineBatch::init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void LineBatch::clear() { verts.clear(); }

void LineBatch::addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    verts.push_back({ a, c });
    verts.push_back({ b, c });
}

void LineBatch::addArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
    addLine(from, to, color);

    glm::vec3 dir = to - from;
    float len = glm::length(dir);
    if (len < 1e-5f) return;
    dir /= len;

    // Build a basis perpendicular to dir for the arrowhead barbs.
    glm::vec3 ref = std::fabs(dir.y) < 0.95f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 right = glm::normalize(glm::cross(dir, ref));
    glm::vec3 up    = glm::normalize(glm::cross(right, dir));

    float head = std::min(0.25f * len, 0.6f);   // head length, capped
    float w    = head * 0.5f;                    // barb half-width
    glm::vec3 base = to - dir * head;
    addLine(to, base + right * w, color);
    addLine(to, base - right * w, color);
    addLine(to, base + up    * w, color);
    addLine(to, base - up    * w, color);
}

void LineBatch::upload() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (verts.size() > capacity) {               // grow (orphan) the buffer
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(LineVertex), verts.data(), GL_DYNAMIC_DRAW);
        capacity = verts.size();
    } else if (!verts.empty()) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(LineVertex), verts.data());
    }
    glBindVertexArray(0);
}

void LineBatch::draw() const {
    if (verts.empty()) return;
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, (GLsizei)verts.size());
    glBindVertexArray(0);
}
