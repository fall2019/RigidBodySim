#pragma once
using namespace VulkanAbstractionLayer;
struct Camera
{
    Vector3 Position{ 40.0f, 30, -40 };
    Vector2 Rotation{ 5.74f, 0.0f };
    float Fov = 65.0f;
    float MovementSpeed = 250.0f;
    float RotationMovementSpeed = 2.5f;
    float AspectRatio = 16.0f / 9.0f;
    float ZNear = 0.1f;
    float ZFar = 100000.0f;

    void Rotate(const Vector2& delta);

    void Move(const Vector3& direction);

    Matrix4x4 GetViewMatrix() const;

    Matrix4x4 GetProjectionMatrix() const;

    Matrix4x4 GetMatrix()const;
};