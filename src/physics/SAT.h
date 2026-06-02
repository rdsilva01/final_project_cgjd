#pragma once
#include <glm/glm.hpp>
#include "geometry/ConvexShape.h"

// Result of a Separating Axis Theorem test between two convex shapes.
struct SATResult {
    bool      intersecting = false;
    glm::vec3 mtvAxis{0.f};   // unit Minimum Translation Vector direction (A -> B)
    float     depth = 0.f;    // penetration depth along mtvAxis
    int       axesTested = 0; // number of candidate axes examined (for reporting)

    // Moving B by (mtvAxis * depth) is the smallest move that separates the pair.
    glm::vec3 mtv() const { return mtvAxis * depth; }
};

// Separating Axis Theorem in 3D for two convex polyhedra.
//
// The pair is disjoint iff there exists an axis on which their projections do
// not overlap. The candidate axes are the face normals of A, the face normals
// of B, and the pairwise cross products of A's and B's edge directions. If all
// axes overlap, the axis of minimum overlap gives the MTV.
SATResult satIntersect(const ConvexShape& A, const ConvexShape& B);
