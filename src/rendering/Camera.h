#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;

    Camera(glm::vec3 position = glm::vec3(0,0,3), glm::vec3 up = glm::vec3(0,1,0),
           float yaw = -90.f, float pitch = 0.f);
    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(Camera_Movement dir, float dt);
    void ProcessMouseMovement(float xoff, float yoff, bool constrainPitch = true);
    void ProcessMouseScroll(float yoff);
private:
    void updateCameraVectors();
};
