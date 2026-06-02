#include "SAT.h"
#include <limits>
#include <cmath>

namespace {
struct Interval { float mn, mx; };

// Project a point set onto a (unit) axis -> [min, max] scalar interval.
Interval project(const std::vector<glm::vec3>& pts, const glm::vec3& axis) {
    float mn =  std::numeric_limits<float>::max();
    float mx = -std::numeric_limits<float>::max();
    for (const auto& p : pts) {
        float d = glm::dot(p, axis);
        mn = std::min(mn, d);
        mx = std::max(mx, d);
    }
    return { mn, mx };
}
} // namespace

SATResult satIntersect(const ConvexShape& A, const ConvexShape& B) {
    SATResult r;

    const std::vector<glm::vec3> va = A.worldVertices();
    const std::vector<glm::vec3> vb = B.worldVertices();
    const std::vector<glm::vec3> na = A.worldFaceNormals();
    const std::vector<glm::vec3> nb = B.worldFaceNormals();
    const std::vector<glm::vec3> ea = A.worldEdgeDirs();
    const std::vector<glm::vec3> eb = B.worldEdgeDirs();

    // Build the candidate axis set: A's faces, B's faces, and edge-edge crosses.
    std::vector<glm::vec3> axes;
    axes.reserve(na.size() + nb.size() + ea.size() * eb.size());
    for (const auto& n : na) axes.push_back(n);
    for (const auto& n : nb) axes.push_back(n);
    for (const auto& da : ea)
        for (const auto& db : eb) {
            glm::vec3 c = glm::cross(da, db);
            float len2 = glm::dot(c, c);
            if (len2 > 1e-8f)                       // skip parallel edge pairs
                axes.push_back(c / std::sqrt(len2));
        }

    r.axesTested = (int)axes.size();

    float     minOverlap =  std::numeric_limits<float>::max();
    glm::vec3 minAxis(0.f);

    for (const auto& axis : axes) {
        Interval ia = project(va, axis);
        Interval ib = project(vb, axis);
        float overlap = std::min(ia.mx, ib.mx) - std::max(ia.mn, ib.mn);
        if (overlap <= 0.f) {                       // found a separating axis
            r.intersecting = false;
            return r;
        }
        if (overlap < minOverlap) {
            minOverlap = overlap;
            minAxis    = axis;
        }
    }

    // No separating axis -> shapes intersect. Min-overlap axis is the MTV.
    r.intersecting = true;
    r.depth        = minOverlap;

    // Orient the axis so it points from A toward B (push B out of A).
    glm::vec3 ab = B.worldCentroid() - A.worldCentroid();
    if (glm::dot(ab, minAxis) < 0.f) minAxis = -minAxis;
    r.mtvAxis = minAxis;

    return r;
}
