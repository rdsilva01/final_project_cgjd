#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0,0,-1)), MovementSpeed(30.f), MouseSensitivity(0.1f), Zoom(45.f) {
    Position = position; WorldUp = up; Yaw = yaw; Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() { return glm::lookAt(Position, Position + Front, Up); }

void Camera::ProcessKeyboard(Camera_Movement dir, float dt) {
    float v = MovementSpeed * dt;
    if (dir == FORWARD)  Position += Front * v;
    if (dir == BACKWARD) Position -= Front * v;
    if (dir == LEFT)     Position -= Right * v;
    if (dir == RIGHT)    Position += Right * v;
}

void Camera::ProcessMouseMovement(float xoff, float yoff, bool constrainPitch) {
    Yaw   += xoff * MouseSensitivity;
    Pitch += yoff * MouseSensitivity;
    if (constrainPitch) { Pitch = glm::clamp(Pitch, -89.f, 89.f); }
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoff) {
    Zoom = glm::clamp(Zoom - yoff, 1.f, 45.f);
}

void Camera::updateCameraVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    f.y = sin(glm::radians(Pitch));
    f.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(f);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
