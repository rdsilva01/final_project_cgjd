#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <utility>
#include "rendering/Mesh.h"

// A convex polyhedron defined generically by its vertices and faces.
//
// SAT only needs three things from any convex shape, all derived here from the
// (vertices, faces) definition so the algorithm never special-cases a shape:
//   * the vertex set            -> projected onto each candidate axis
//   * the unique face normals   -> candidate axes (potential separating planes)
//   * the unique edge directions-> crossed pairwise to form edge-edge axes
//
// Local data is rotated/translated into world space on demand via the
// position + orientation transform.
class ConvexShape {
public:
    // --- local-space definition (filled by a factory, then finalize()) ---
    std::vector<glm::vec3>        vertices;   // local vertices
    std::vector<std::vector<int>> faces;      // each face: CCW vertex indices (outward)

    // --- derived in local space by finalize() ---
    std::vector<glm::vec3>            faceNormals;   // unique outward normals
    std::vector<glm::vec3>            edgeDirs;      // unique edge directions
    std::vector<std::pair<int,int>>   edgeList;      // unique edges (for wireframe)
    glm::vec3                         localCentroid{0.f};
    Mesh                              mesh;          // triangulated, per-face normals

    // --- world transform ---
    glm::vec3 position{0.f};
    glm::quat orientation{1.f, 0.f, 0.f, 0.f};

    // Compute faceNormals, edgeDirs, edgeList, centroid and the render mesh.
    void finalize();
    void setupBuffers() { mesh.setupBuffers(); }
    void draw() const   { mesh.draw(); }

    glm::mat4 modelMatrix() const;

    // World-space accessors used by SAT.
    std::vector<glm::vec3> worldVertices()    const; // rotate + translate
    std::vector<glm::vec3> worldFaceNormals() const; // rotate only (unit preserved)
    std::vector<glm::vec3> worldEdgeDirs()    const; // rotate only
    glm::vec3              worldCentroid()    const;
};

// Factories — each centred on its centroid at the local origin.
ConvexShape makeBox(glm::vec3 halfExtents);
ConvexShape makeTetrahedron(float radius);
ConvexShape makeOctahedron(float radius);
