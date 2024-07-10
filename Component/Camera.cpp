#include "Camera.h"
#include "qmath.h"

Camera::Camera(QVector3D position, QVector3D up, float yaw, float pitch)
    : Front(QVector3D(0.0f, 0.0f, -1.0f)), 
    MovementSpeed(SPEED), 
    MouseSensitivity(SENSITIVITY), 
    Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// constructor with scalar values

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(QVector3D(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = QVector3D(posX, posY, posZ);
    WorldUp = QVector3D(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix

QMatrix4x4 Camera::GetViewMatrix()
{
    QMatrix4x4 view;
    view.lookAt(Position, Position + Front, Up);
    return view;
}

QMatrix3x3 Camera::GetViewMatrix3x3()
{
    QMatrix3x3 result;
    QMatrix4x4 view = GetViewMatrix();

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            result(row, col) = view(row, col);
        }
    }

    return result;
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 90.0f)
        Zoom = 90.0f;
}

// calculates the front vector from the Camera's (updated) Euler Angles

void Camera::updateCameraVectors()
{
    // calculate the new Front vector
    QVector3D front;
    front.setX(cos(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
    front.setY(sin(qDegreesToRadians(Pitch)));
    front.setZ(sin(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
    front.normalize();
    Front = front;
    // also re-calculate the Right and Up vector
    Right = QVector3D::crossProduct(Front, WorldUp);
    Right.normalize();
    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = QVector3D::crossProduct(Right, Front);
    Up.normalize();
}
