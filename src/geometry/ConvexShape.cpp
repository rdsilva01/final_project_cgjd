#include "ConvexShape.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace {
// Two axes are "the same" for SAT if they are parallel OR anti-parallel,
// since a separating plane direction n and -n test identically. Keeping the
// axis set minimal avoids redundant projections.
bool sameAxis(const glm::vec3& a, const glm::vec3& b) {
    float d = glm::dot(a, b);                 // a, b assumed unit length
    return std::fabs(std::fabs(d) - 1.f) < 1e-5f;
}

void addUniqueAxis(std::vector<glm::vec3>& axes, const glm::vec3& v) {
    float len2 = glm::dot(v, v);
    if (len2 < 1e-12f) return;                // degenerate
    glm::vec3 n = v / std::sqrt(len2);
    for (const auto& a : axes)
        if (sameAxis(a, n)) return;
    axes.push_back(n);
}
} // namespace

void ConvexShape::finalize() {
    faceNormals.clear();
    edgeDirs.clear();
    edgeList.clear();
    mesh.vertices.clear();
    mesh.indices.clear();

    // Centroid (average of vertices — exact for the symmetric factory shapes).
    glm::vec3 c(0.f);
    for (const auto& v : vertices) c += v;
    localCentroid = vertices.empty() ? glm::vec3(0.f) : c / float(vertices.size());

    // Edge bookkeeping: undirected pairs, deduplicated.
    auto addEdge = [&](int i, int j) {
        int a = std::min(i, j), b = std::max(i, j);
        for (const auto& e : edgeList)
            if (e.first == a && e.second == b) return;
        edgeList.emplace_back(a, b);
        addUniqueAxis(edgeDirs, vertices[b] - vertices[a]);
    };

    for (const auto& f : faces) {
        if (f.size() < 3) continue;

        // Face normal from the first corner. Orient it outward using the
        // centroid so lighting is correct regardless of the hand-written
        // winding (SAT itself is sign-agnostic, but rendering is not).
        glm::vec3 n = glm::normalize(
            glm::cross(vertices[f[1]] - vertices[f[0]],
                       vertices[f[2]] - vertices[f[0]]));
        glm::vec3 fc(0.f);
        for (int idx : f) fc += vertices[idx];
        fc /= float(f.size());
        if (glm::dot(n, fc - localCentroid) < 0.f) n = -n;
        addUniqueAxis(faceNormals, n);

        // Edges of this face.
        for (size_t k = 0; k < f.size(); ++k)
            addEdge(f[k], f[(k + 1) % f.size()]);

        // Triangulate as a fan with per-face normal (vertices duplicated per face
        // so shared corners can carry distinct face normals -> flat shading).
        unsigned int base = (unsigned int)mesh.vertices.size();
        for (int idx : f) mesh.vertices.push_back({ vertices[idx], n });
        for (size_t k = 1; k + 1 < f.size(); ++k)
            mesh.indices.insert(mesh.indices.end(),
                                { base, base + (unsigned int)k, base + (unsigned int)k + 1 });
    }
}

glm::mat4 ConvexShape::modelMatrix() const {
    glm::mat4 m = glm::translate(glm::mat4(1.f), position);
    m *= glm::mat4_cast(orientation);
    return m;
}

std::vector<glm::vec3> ConvexShape::worldVertices() const {
    std::vector<glm::vec3> out;
    out.reserve(vertices.size());
    for (const auto& v : vertices) out.push_back(position + orientation * v);
    return out;
}

std::vector<glm::vec3> ConvexShape::worldFaceNormals() const {
    std::vector<glm::vec3> out;
    out.reserve(faceNormals.size());
    for (const auto& n : faceNormals) out.push_back(orientation * n);
    return out;
}

std::vector<glm::vec3> ConvexShape::worldEdgeDirs() const {
    std::vector<glm::vec3> out;
    out.reserve(edgeDirs.size());
    for (const auto& e : edgeDirs) out.push_back(orientation * e);
    return out;
}

glm::vec3 ConvexShape::worldCentroid() const {
    return position + orientation * localCentroid;
}

// ---- factories --------------------------------------------------------------

ConvexShape makeBox(glm::vec3 h) {
    ConvexShape s;
    s.vertices = {
        {-h.x,-h.y,-h.z}, { h.x,-h.y,-h.z}, { h.x,-h.y, h.z}, {-h.x,-h.y, h.z}, // 0-3 bottom
        {-h.x, h.y,-h.z}, { h.x, h.y,-h.z}, { h.x, h.y, h.z}, {-h.x, h.y, h.z}, // 4-7 top
    };
    // Each face CCW when viewed from outside.
    s.faces = {
        {0,3,2,1}, // bottom -Y
        {4,5,6,7}, // top    +Y
        {3,7,6,2}, // front  +Z
        {0,1,5,4}, // back   -Z
        {1,2,6,5}, // right  +X
        {0,4,7,3}, // left   -X
    };
    s.finalize();
    return s;
}

ConvexShape makeTetrahedron(float r) {
    ConvexShape s;
    // Regular tetrahedron vertices on a cube's alternating corners, scaled to radius r.
    float a = r / std::sqrt(3.f);
    s.vertices = {
        { a,  a,  a}, // 0
        { a, -a, -a}, // 1
        {-a,  a, -a}, // 2
        {-a, -a,  a}, // 3
    };
    // CCW from outside (verified so normals point away from centroid at origin).
    s.faces = {
        {0,1,2},
        {0,3,1},
        {0,2,3},
        {1,3,2},
    };
    s.finalize();
    return s;
}

ConvexShape makeOctahedron(float r) {
    ConvexShape s;
    s.vertices = {
        { r, 0, 0}, {-r, 0, 0}, // 0,1  ±X
        { 0, r, 0}, { 0,-r, 0}, // 2,3  ±Y
        { 0, 0, r}, { 0, 0,-r}, // 4,5  ±Z
    };
    // 8 triangular faces, CCW from outside.
    s.faces = {
        {0,2,4}, {2,1,4}, {1,3,4}, {3,0,4}, // upper/lower around +Z apex
        {2,0,5}, {1,2,5}, {3,1,5}, {0,3,5}, // around -Z apex
    };
    s.finalize();
    return s;
}
