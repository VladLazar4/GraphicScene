#include "Camera.hpp"
#include <iostream>

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return  glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraFrontDirection, this->cameraUpDirection);
    }

    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;

    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        if (direction == MOVE_FORWARD)
            this->cameraPosition += speed * this->cameraFrontDirection;
        if (direction == MOVE_BACKWARD)
            this->cameraPosition -= speed * this->cameraFrontDirection;
        if (direction == MOVE_LEFT)
            this->cameraPosition -= glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
        if (direction == MOVE_RIGHT)
            this->cameraPosition += glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
        if (direction == MOVE_UP)
            this->cameraPosition += -1 * speed * this->cameraUpDirection;
        if (direction == MOVE_DOWN)
            this->cameraPosition += speed * this->cameraUpDirection;
        std::cout << this->cameraPosition.x << " " << this->cameraPosition.y << " " << this->cameraPosition.z << '\n';
    }


    void Camera::rotate(float pitch, float yaw) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraFrontDirection = glm::normalize(front);
    }

    void Camera::animation(float coordinate_x, float coordinate_y, float coordinate_z, float ct_coordinate_x, float ct_coordinate_y, float ct_coordinate_z, float angle) {
        //-18.0296 39.9256 52.8063
        //29.5867 32.2316 19.9507

        //-92.2354 34.3752 49.999
        //-88.0093 17.8384 60.175

        this->cameraPosition = glm::vec3(ct_coordinate_x, ct_coordinate_y, ct_coordinate_z);
        //20.6856 2.23277 -38.1164
        //0.182253 1.74479 -33.6512
        this->cameraTarget = glm::vec3(3.6856f + ct_coordinate_x, 3.0f + ct_coordinate_y, -15.1164f + ct_coordinate_z);

        glm::mat4 camera = glm::mat4(1.0f);

        camera = glm::rotate(camera, glm::radians(angle), glm::vec3(0, 1, 0));

        this->cameraPosition = glm::vec4(camera * glm::vec4(this->cameraPosition, 1));
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        
        camera = glm::translate(camera, glm::vec3(ct_coordinate_x + coordinate_x, ct_coordinate_y + coordinate_y, ct_coordinate_z + coordinate_z));

        this->cameraPosition = glm::vec4(camera * glm::vec4(this->cameraPosition, 1));
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
}