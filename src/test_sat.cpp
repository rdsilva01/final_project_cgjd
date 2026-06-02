// Standalone numerical sanity check for the SAT core (no OpenGL needed).
// Build:  see the command in updates.md / the comment at the bottom.
#include "geometry/ConvexShape.h"
#include "physics/SAT.h"
#include <glm/gtc/quaternion.hpp>
#include <cstdio>
#include <cmath>

static int failures = 0;

static void check(bool cond, const char* msg) {
    std::printf("  [%s] %s\n", cond ? "PASS" : "FAIL", msg);
    if (!cond) ++failures;
}

static bool approx(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) < eps; }

int main() {
    std::printf("SAT core tests\n");

    // 1. Two axis-aligned boxes (half-extent 1) far apart -> disjoint.
    {
        ConvexShape a = makeBox(glm::vec3(1.f));
        ConvexShape b = makeBox(glm::vec3(1.f));
        b.position = glm::vec3(5.f, 0.f, 0.f);
        SATResult r = satIntersect(a, b);
        check(!r.intersecting, "boxes 5 apart are disjoint");
    }

    // 2. Boxes overlapping along X: centres 1.5 apart, each half-extent 1 ->
    //    overlap = (1+1) - 1.5 = 0.5 along +X.
    {
        ConvexShape a = makeBox(glm::vec3(1.f));
        ConvexShape b = makeBox(glm::vec3(1.f));
        b.position = glm::vec3(1.5f, 0.f, 0.f);
        SATResult r = satIntersect(a, b);
        check(r.intersecting, "boxes 1.5 apart intersect");
        check(approx(r.depth, 0.5f), "penetration depth == 0.5");
        check(approx(std::fabs(r.mtvAxis.x), 1.f) && approx(r.mtvAxis.y, 0.f) && approx(r.mtvAxis.z, 0.f),
              "MTV axis is along X");
        check(r.mtvAxis.x > 0.f, "MTV points A->B (+X)");
    }

    // 3. Just-touching (faces coincident) counts as NOT intersecting (overlap == 0).
    {
        ConvexShape a = makeBox(glm::vec3(1.f));
        ConvexShape b = makeBox(glm::vec3(1.f));
        b.position = glm::vec3(2.f, 0.f, 0.f);
        SATResult r = satIntersect(a, b);
        check(!r.intersecting, "exactly touching boxes are not intersecting");
    }

    // 4. A 45-degree rotated box vs an axis-aligned box: the diagonal reaches
    //    further (half-diagonal = sqrt(2) ~ 1.414), so they intersect at a gap
    //    where two AABBs would not.
    {
        ConvexShape a = makeBox(glm::vec3(1.f));
        ConvexShape b = makeBox(glm::vec3(1.f));
        b.orientation = glm::angleAxis(glm::radians(45.f), glm::vec3(0, 0, 1));
        b.position = glm::vec3(2.2f, 0.f, 0.f);   // 2.2 < 1 + 1.414
        SATResult r = satIntersect(a, b);
        check(r.intersecting, "rotated box reaches across the diagonal");
    }

    // 5. Symmetry: swapping A and B flips the MTV direction but keeps the depth.
    {
        ConvexShape a = makeBox(glm::vec3(1.f));
        ConvexShape b = makeTetrahedron(2.f);
        b.position = glm::vec3(1.0f, 0.2f, 0.f);
        SATResult ab = satIntersect(a, b);
        SATResult ba = satIntersect(b, a);
        check(ab.intersecting == ba.intersecting, "intersect result is symmetric");
        if (ab.intersecting && ba.intersecting) {
            check(approx(ab.depth, ba.depth), "depth is symmetric");
            check(approx(glm::dot(ab.mtvAxis, ba.mtvAxis), -1.f, 1e-2f), "MTV axis flips when swapped");
        }
    }

    // 6. Tetrahedron vs box, clearly separated.
    {
        ConvexShape a = makeBox(glm::vec3(1.f));
        ConvexShape b = makeTetrahedron(1.f);
        b.position = glm::vec3(0.f, 6.f, 0.f);
        SATResult r = satIntersect(a, b);
        check(!r.intersecting, "tetra far above box is disjoint");
    }

    std::printf("\n%s (%d failure%s)\n", failures ? "TESTS FAILED" : "ALL TESTS PASSED",
                failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
