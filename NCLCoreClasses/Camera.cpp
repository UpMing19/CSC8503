#include "Camera.h"
#include "Window.h"
#include <algorithm>

using namespace NCL;

/*
Polls the camera for keyboard / mouse movement.
Should be done once per frame! Pass it the msec since
last frame (default value is for simplicities sake...)
*/
void Camera::UpdateCamera(float dt)
{
    if (!enableInput)
        return;

    //Update the mouse by how much
    pitch	-= (Window::GetMouse()->GetRelativePosition().y);
    yaw		-= (Window::GetMouse()->GetRelativePosition().x);

    //Bounds check the pitch, to be between straight up and straight down ;)
    pitch = std::min(pitch, 90.0f);
    pitch = std::max(pitch, -90.0f);

    if (yaw <0) {
        yaw += 360.0f;
    }
    if (yaw > 360.0f) {
        yaw -= 360.0f;
    }

    float frameSpeed = defaultSpeed * dt;

    Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Matrix4::Rotation(pitch, Vector3(1, 0, 0));
    camForward = rotation * Vector3(0, 0, -1);
    camRight = rotation * Vector3(1, 0, 0);
    camUp = rotation * Vector3(0, 1, 0);

    if (Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT))
        frameSpeed *= 2.0f;

    if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
        position += camForward * frameSpeed;
    }
    if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
        position -= camForward * frameSpeed;
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
        position -= camRight * frameSpeed;
    }
    if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
        position += camRight * frameSpeed;
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE)) {
        position.y += frameSpeed;
    }
    if (Window::GetKeyboard()->KeyDown(KeyCodes::CONTROL)) {
        position.y -= frameSpeed;
    }
}

/*
Generates a view matrix for the camera's viewpoint. This matrix can be sent
straight to the shader...it's already an 'inverse camera' matrix.
*/
Matrix4 Camera::BuildViewMatrix() const {
    //Why do a complicated matrix inversion, when we can just generate the matrix
    //using the negative values ;). The matrix multiplication order is important!
    return	Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
              Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
              Matrix4::Translation(-position);
};

Matrix4 Camera::BuildProjectionMatrix(float currentAspect) const {
    if (camType == CameraType::Orthographic) {
        return Matrix4::Orthographic(left, right, bottom, top, nearPlane, farPlane);
    }
    //else if (camType == CameraType::Perspective) {
    return Matrix4::Perspective(nearPlane, farPlane, currentAspect, fov);
    //}
}

Camera Camera::BuildPerspectiveCamera(const Vector3& pos, float pitch, float yaw, float fov, float nearPlane, float farPlane) {
    Camera c;
    c.camType	= CameraType::Perspective;
    c.position	= pos;
    c.pitch		= pitch;
    c.yaw		= yaw;
    c.nearPlane = nearPlane;
    c.farPlane  = farPlane;

    c.fov		= fov;

    return c;
}
Camera Camera::BuildOrthoCamera(const Vector3& pos, float pitch, float yaw, float left, float right, float top, float bottom, float nearPlane, float farPlane) {
    Camera c;
    c.camType	= CameraType::Orthographic;
    c.position	= pos;
    c.pitch		= pitch;
    c.yaw		= yaw;
    c.nearPlane = nearPlane;
    c.farPlane	= farPlane;

    c.left		= left;
    c.right		= right;
    c.top		= top;
    c.bottom	= bottom;

    return c;
}