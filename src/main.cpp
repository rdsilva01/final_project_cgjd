#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "rendering/Camera.h"
#include "rendering/Shader.h"
#include "rendering/Mesh.h"
#include "rendering/LineBatch.h"
#include "geometry/ConvexShape.h"
#include "physics/SAT.h"

#include <iostream>
#include <cstdio>
#include <cmath>

// ---- window -----------------------------------------------------------------
const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;
float deltaTime = 0.f, lastFrame = 0.f;

// ---- camera -----------------------------------------------------------------
Camera camera(glm::vec3(0.f, 5.f, 20.f), glm::vec3(0, 1, 0), -90.f, -12.f);
float lastX = SCR_WIDTH / 2.f, lastY = SCR_HEIGHT / 2.f;
bool  firstMouse = true;

// ---- state ------------------------------------------------------------------
bool freezeMouse = false;   // SPACE: release cursor to interact with window
bool showWire    = true;    // N
bool showMTV     = true;    // M
bool translucent = false;   // T
bool applyMTVReq = false;   // P: push B out of A by the MTV (handled in the loop)

enum ShapeType { BOX, TETRA, OCTA };
const char* shapeName(ShapeType t) { return t == BOX ? "box" : t == TETRA ? "tetrahedron" : "octahedron"; }

ConvexShape shapeA, shapeB;          // A static, B controlled by the user
ShapeType   typeA = BOX, typeB = TETRA;

// ---- helpers ----------------------------------------------------------------
static ConvexShape makeShape(ShapeType t) {
    switch (t) {
        case BOX:   return makeBox(glm::vec3(2.5f));
        case TETRA: return makeTetrahedron(3.4f);
        case OCTA:  return makeOctahedron(3.4f);
    }
    return makeBox(glm::vec3(2.5f));
}

static void freeMesh(Mesh& m) {
    if (m.vao) glDeleteVertexArrays(1, &m.vao);
    if (m.vbo) glDeleteBuffers(1, &m.vbo);
    if (m.ebo) glDeleteBuffers(1, &m.ebo);
    m.vao = m.vbo = m.ebo = 0;
}

// Rebuild a shape to a new type, preserving its world transform.
static void setShape(ConvexShape& s, ShapeType t) {
    glm::vec3 p = s.position;
    glm::quat o = s.orientation;
    freeMesh(s.mesh);
    s = makeShape(t);
    s.position = p;
    s.orientation = o;
    s.setupBuffers();
}

static void resetB() {
    shapeB.position    = glm::vec3(6.f, 0.f, 0.f);
    shapeB.orientation = glm::quat(1.f, 0.f, 0.f, 0.f);
}

static void addWire(LineBatch& lb, const ConvexShape& s, glm::vec3 color, glm::vec3 offset = glm::vec3(0.f)) {
    std::vector<glm::vec3> wv = s.worldVertices();
    for (const auto& e : s.edgeList)
        lb.addLine(wv[e.first] + offset, wv[e.second] + offset, color);
}

// ---- GLFW callbacks ---------------------------------------------------------
void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

void mouse_callback(GLFWwindow*, double xIn, double yIn) {
    float x = (float)xIn, y = (float)yIn;
    if (firstMouse) { lastX = x; lastY = y; firstMouse = false; }
    if (!freezeMouse) camera.ProcessMouseMovement(x - lastX, lastY - y);
    lastX = x; lastY = y;
}

void scroll_callback(GLFWwindow*, double, double yo) { camera.ProcessMouseScroll((float)yo); }

void key_callback(GLFWwindow* window, int key, int, int action, int mods) {
    if (action != GLFW_PRESS) return;
    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    switch (key) {
        case GLFW_KEY_SPACE:
            freezeMouse = !freezeMouse;
            glfwSetInputMode(window, GLFW_CURSOR, freezeMouse ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            break;
        case GLFW_KEY_N: showWire    = !showWire;    break;
        case GLFW_KEY_M: showMTV     = !showMTV;     break;
        case GLFW_KEY_T: translucent = !translucent; break;
        case GLFW_KEY_P: applyMTVReq = true;         break;
        case GLFW_KEY_0:
        case GLFW_KEY_BACKSPACE: resetB(); break;
        case GLFW_KEY_1:
            if (shift) { typeA = BOX;   setShape(shapeA, typeA); }
            else       { typeB = BOX;   setShape(shapeB, typeB); }
            break;
        case GLFW_KEY_2:
            if (shift) { typeA = TETRA; setShape(shapeA, typeA); }
            else       { typeB = TETRA; setShape(shapeB, typeB); }
            break;
        case GLFW_KEY_3:
            if (shift) { typeA = OCTA;  setShape(shapeA, typeA); }
            else       { typeB = OCTA;  setShape(shapeB, typeB); }
            break;
        default: break;
    }
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    // Fly camera (only while the cursor is captured).
    if (!freezeMouse) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD,  deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT,     deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT,    deltaTime);
    }

    // Translate shape B along world axes.
    float mv = 6.f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) shapeB.position.x -= mv;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) shapeB.position.x += mv;
    if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) shapeB.position.z -= mv;
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) shapeB.position.z += mv;
    if (glfwGetKey(window, GLFW_KEY_R)     == GLFW_PRESS) shapeB.position.y += mv;
    if (glfwGetKey(window, GLFW_KEY_F)     == GLFW_PRESS) shapeB.position.y -= mv;

    // Rotate shape B about world axes (left-multiply).
    float rot = 1.6f * deltaTime;
    auto spin = [&](float ang, glm::vec3 axis) {
        shapeB.orientation = glm::normalize(glm::angleAxis(ang, axis) * shapeB.orientation);
    };
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) spin( rot, glm::vec3(1, 0, 0));
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) spin(-rot, glm::vec3(1, 0, 0));
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) spin( rot, glm::vec3(0, 1, 0));
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) spin(-rot, glm::vec3(0, 1, 0));
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) spin( rot, glm::vec3(0, 0, 1));
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) spin(-rot, glm::vec3(0, 0, 1));
}

// ---- main -------------------------------------------------------------------
int main() {
    std::cout <<
        "\n  Separating Axis Theorem (SAT) in 3D\n"
        "  ------------------------------------\n"
        "  Camera   WASD + mouse, scroll zoom, SPACE release/grab cursor\n"
        "  Move B   arrows (X/Z), R/F (up/down)\n"
        "  Rotate B I/K pitch, J/L yaw, U/O roll\n"
        "  Shape B  1 box   2 tetra   3 octa   (hold SHIFT to change shape A)\n"
        "  Resolve  P apply the MTV (push B out of A)\n"
        "  Toggles  N wireframe, M MTV arrow, T translucency, 0 reset B\n"
        "  ESC quit\n\n"
        "  Red = intersecting, the yellow arrow is the Minimum Translation Vector;\n"
        "  the green ghost shows where B would sit once pushed out by the MTV.\n\n";

    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SAT 3D", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD failed\n"; return -1;
    }

    camera.MovementSpeed = 9.f;

    // Scene: A static at origin, B starts to the right and overlapping range.
    shapeA = makeShape(typeA);
    shapeB = makeShape(typeB);
    resetB();
    shapeA.setupBuffers();
    shapeB.setupBuffers();

    LineBatch lines;
    lines.init();

    Shader litShader ("shaders/lit.vert",  "shaders/lit.frag");
    Shader flatShader("shaders/flat.vert", "shaders/flat.frag");

    glm::vec3 lightPos(12.f, 18.f, 16.f);
    float titleTimer = 0.f;

    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame; lastFrame = now;

        processInput(window);

        // ---- collision test --------------------------------------------------
        SATResult sat = satIntersect(shapeA, shapeB);

        // Apply the MTV on request: translate B out of A along the MTV (plus a
        // small epsilon for a clean separation), then re-test so this frame
        // already shows the resolved, non-intersecting state.
        if (applyMTVReq) {
            if (sat.intersecting)
                shapeB.position += sat.mtvAxis * (sat.depth + 0.01f);
            applyMTVReq = false;
            sat = satIntersect(shapeA, shapeB);
        }

        glm::vec3 colorA = sat.intersecting ? glm::vec3(0.88f, 0.32f, 0.28f) : glm::vec3(0.32f, 0.55f, 0.85f);
        glm::vec3 colorB = sat.intersecting ? glm::vec3(0.95f, 0.58f, 0.30f) : glm::vec3(0.40f, 0.80f, 0.52f);

        // ---- matrices --------------------------------------------------------
        // Use the real framebuffer size (in pixels) so the viewport fills the
        // whole window on HiDPI/Retina displays, where it is 2x the window size.
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        float aspect = fbH > 0 ? (float)fbW / (float)fbH : 1.f;

        glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 500.f);
        glm::mat4 view = camera.GetViewMatrix();

        glViewport(0, 0, fbW, fbH);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_CULL_FACE);
        glClearColor(0.10f, 0.11f, 0.13f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---- solid shapes ----------------------------------------------------
        litShader.use();
        litShader.setMat4("view", view);
        litShader.setMat4("projection", proj);
        litShader.setVec3("lightPos", lightPos);
        litShader.setVec3("viewPos", camera.Position);

        if (translucent) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            litShader.setFloat("alpha", 0.45f);
        } else {
            litShader.setFloat("alpha", 1.0f);
        }

        litShader.setMat4("model", shapeA.modelMatrix());
        litShader.setVec3("solidColor", colorA);
        shapeA.draw();

        litShader.setMat4("model", shapeB.modelMatrix());
        litShader.setVec3("solidColor", colorB);
        shapeB.draw();

        if (translucent) { glDisable(GL_BLEND); glDepthMask(GL_TRUE); }

        // ---- lines: grid, wireframe, MTV ------------------------------------
        lines.clear();

        // ground grid on the XZ plane below the shapes
        const float G = 20.f; const int N = 20;
        glm::vec3 gridCol(0.24f, 0.24f, 0.30f);
        for (int i = -N; i <= N; i += 2) {
            lines.addLine({(float)i, -4.f, -G}, {(float)i, -4.f, G}, gridCol);
            lines.addLine({-G, -4.f, (float)i}, {G, -4.f, (float)i}, gridCol);
        }

        if (showWire) {
            glm::vec3 wireCol(0.85f, 0.86f, 0.92f);
            addWire(lines, shapeA, wireCol);
            addWire(lines, shapeB, wireCol);
        }

        if (sat.intersecting && showMTV) {
            // Faithful MTV arrow from B's centroid; green ghost = B pushed out.
            glm::vec3 c = shapeB.worldCentroid();
            lines.addArrow(c, c + sat.mtv(), glm::vec3(1.0f, 0.92f, 0.15f));
            addWire(lines, shapeB, glm::vec3(0.30f, 1.0f, 0.45f), sat.mtv());
        }

        lines.upload();
        flatShader.use();
        flatShader.setMat4("view", view);
        flatShader.setMat4("projection", proj);
        lines.draw();

        // ---- HUD in the title bar (throttled) -------------------------------
        titleTimer += deltaTime;
        if (titleTimer > 0.1f) {
            titleTimer = 0.f;
            char buf[256];
            if (sat.intersecting)
                std::snprintf(buf, sizeof(buf),
                    "SAT 3D | A:%s  B:%s | INTERSECTING  depth=%.3f  axis=(%.2f,%.2f,%.2f) | %d axes",
                    shapeName(typeA), shapeName(typeB), sat.depth,
                    sat.mtvAxis.x, sat.mtvAxis.y, sat.mtvAxis.z, sat.axesTested);
            else
                std::snprintf(buf, sizeof(buf),
                    "SAT 3D | A:%s  B:%s | CLEAR (separating axis found) | %d axes",
                    shapeName(typeA), shapeName(typeB), sat.axesTested);
            glfwSetWindowTitle(window, buf);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
