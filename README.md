# final_project_cgjd — SAT Algorithm in 3D

An interactive 3D implementation of the **Separating Axis Theorem (SAT)** for
collision detection between **arbitrary convex polyhedra**, including the
**Minimum Translation Vector (MTV)** for resolving penetration.

SAT is a single generic test that works for any convex shape pair, removing the
need for per-shape-pair collision code. A pair of convex shapes is disjoint *iff*
there is an axis on which their projections do not overlap. In 3D the candidate
axes are: the face normals of A, the face normals of B, and the pairwise cross
products of A's and B's edge directions. If every axis overlaps, the axis of
minimum overlap gives the MTV.

## Build & run

```sh
cmake -S . -B build      # fetches GLFW + GLM the first time
cmake --build build -j4
./build/SAT3D
```

### Headless correctness tests (no OpenGL)

```sh
cmake --build build --target sat_tests && ./build/sat_tests
```

## Controls

| Input                 | Action                                            |
|-----------------------|---------------------------------------------------|
| `W A S D` + mouse     | Fly camera, scroll to zoom                        |
| `SPACE`               | Release / grab the mouse cursor                   |
| Arrow keys            | Move shape **B** in X / Z                          |
| `R` / `F`             | Move shape **B** up / down (Y)                     |
| `I`/`K` `J`/`L` `U`/`O` | Rotate shape **B** (pitch / yaw / roll)         |
| `1` `2` `3`           | Set shape **B** = box / tetrahedron / octahedron  |
| `SHIFT` + `1`/`2`/`3` | Set shape **A** instead                           |
| `N` / `M` / `T`       | Toggle wireframe / MTV arrow / translucency       |
| `0` or `Backspace`    | Reset shape **B**                                 |
| `ESC`                 | Quit                                              |

Shapes turn **red** when intersecting. The **yellow arrow** is the MTV; the
**green ghost** shows where B would sit once pushed out by the MTV. The window
title shows the live intersection state, penetration depth, MTV axis, and the
number of candidate axes tested.

## Layout

```
src/geometry/ConvexShape.{h,cpp}   generic convex polyhedron (verts/faces -> axes)
src/physics/SAT.{h,cpp}            the SAT test + MTV
src/rendering/{Camera,Shader,Mesh,LineBatch}
src/test_sat.cpp                   headless numerical tests for the SAT core
shaders/lit.* shaders/flat.*       Phong solids / flat-coloured lines
```

Reference: <https://dyn4j.org/2010/01/sat/>
