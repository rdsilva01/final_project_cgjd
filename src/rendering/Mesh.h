#pragma once
#include <glm/glm.hpp>
#include <vector>

struct MeshVertex { glm::vec3 position, normal; };

// A simple indexed triangle mesh with position+normal vertices.
// Geometry is uploaded once; the per-frame transform/colour are passed as
// shader uniforms by the caller, so the same mesh can be re-drawn cheaply.
class Mesh {
public:
    std::vector<MeshVertex>   vertices;
    std::vector<unsigned int> indices;
    unsigned int vao = 0, vbo = 0, ebo = 0;

    void setupBuffers();
    void draw() const;
};
