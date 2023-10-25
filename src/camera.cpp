#include "VulkanAbstractionLayer/VectorMath.h"

#include "camera.h"

#include <algorithm>

void Camera::Rotate(const VulkanAbstractionLayer::Vector2& delta)
{
    Rotation += RotationMovementSpeed * delta;

    constexpr float MaxAngleY = HalfPi - 0.001f;
    constexpr float MaxAngleX = TwoPi;
    Rotation.y = std::clamp(Rotation.y, -MaxAngleY, MaxAngleY);
    Rotation.x = std::fmod(Rotation.x, MaxAngleX);
}

void Camera::Move(const Vector3& direction)
{
    Matrix3x3 view{
        std::sin(Rotation.x), 0.0f, std::cos(Rotation.x), // forward
        0.0f, 1.0f, 0.0f, // up
        std::sin(Rotation.x - HalfPi), 0.0f, std::cos(Rotation.x - HalfPi) // right
    };

    Position += MovementSpeed * (view * direction);
}

Matrix4x4 Camera::GetViewMatrix() const
{
    Vector3 direction{
        std::cos(Rotation.y) * std::sin(Rotation.x),
        std::sin(Rotation.y),
        std::cos(Rotation.y) * std::cos(Rotation.x)
    };
    return MakeLookAtMatrix(Position, direction, Vector3{0.0f, 1.0f, 0.0f});
}

Matrix4x4 Camera::GetProjectionMatrix() const
{
    return MakePerspectiveMatrix(ToRadians(Fov), AspectRatio, ZNear, ZFar);
}

Matrix4x4 Camera::GetMatrix()const 
{
    return GetProjectionMatrix() * GetViewMatrix();
}