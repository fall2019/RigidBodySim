#include "physicsPass.h"
#include "glm/gtx/matrix_cross_product.hpp"
#include "VulkanAbstractionLayer/Window.h"

RigidBodyDynamicsPass::RigidBodyDynamicsPass(SharedResources& sr)
    : sharedResources(sr)
{

}
void RigidBodyDynamicsPass::SetupPipeline(PipelineState pipeline)
{

}
void RigidBodyDynamicsPass::ResolveResources(ResolveState resolve)
{

}
void RigidBodyDynamicsPass::BeforeRender(RenderPassState state)
{
    ImGui::Begin("Model");
    ImGui::DragFloat3("Input Linear Velocity", &inputLinearVelocity[0], 0.01f);
    ImGui::DragFloat3("Input Angular Velocity", &inputAngularVelocity[0], 0.01f);
    ImGui::DragFloat3("Input Position", &inputPosition[0], 0.01f);
    ImGui::DragFloat3("Input Euler Angle", &inputEulerAngle[0], 0.01f);
    ImGui::End();

    if (ImGui::IsKeyDown((int)KeyCode::R))
    {
        sharedResources.teapot.position = inputPosition;
        sharedResources.teapot.orientation = glm::quat(inputEulerAngle);
        sharedResources.teapot.w = inputAngularVelocity;
        sharedResources.teapot.v = inputLinearVelocity;
    }
}
void RigidBodyDynamicsPass::OnRender(RenderPassState state)
{
    UpdateVelocity();
    UpdateAngular();
    for (const auto& it : sharedResources.planes)
        SolveCollision(it);
    sharedResources.teapot.position = UpdatePosition(sharedResources.teapot.position);
    sharedResources.teapot.orientation = UpdateRotation(sharedResources.teapot.orientation);
}
void RigidBodyDynamicsPass::UpdateVelocity()
{
    sharedResources.teapot.v += dt * gravity;
    sharedResources.teapot.v *= linearDecay;
}
Vector3 RigidBodyDynamicsPass::UpdatePosition(Vector3 x) const
{
    return x + sharedResources.teapot.v * dt;
}
glm::quat RigidBodyDynamicsPass::UpdateRotation(glm::quat q)
{
    glm::quat qNext = glm::quat(0, Vector3(sharedResources.teapot.w * dt / 2.0f)) * q;
    qNext += q;
    return glm::normalize(qNext);
}
void RigidBodyDynamicsPass::UpdateAngular()
{
    sharedResources.teapot.w *= angularDecay;
}
bool RigidBodyDynamicsPass::CollideWithPlane(const PlaneGeometry& plane, Vector3 x) const
{
    return glm::dot((x - plane.position), plane.normal) < 0;
}
void RigidBodyDynamicsPass::SolveCollision(const PlaneGeometry& plane)
{
    int num = 0;
    Vector3 X{ 0,0,0 };
    const auto orientation = glm::mat4_cast(sharedResources.teapot.orientation);
    //Todo: Make this iteration parallel in compute shader.
    for (auto& shape : sharedResources.teapot.modelData.Shapes)
    {
        for (auto& vert : shape.Vertices)
        {
            Vector3 localPos = Vector3(orientation * glm::vec4(vert.Position, 1.0));
            Vector3 xi = localPos + sharedResources.teapot.position;
            Vector3 vi = sharedResources.teapot.v + cross(sharedResources.teapot.w, localPos);
            if (!CollideWithPlane(plane, xi))
                continue;
            if (dot(vi, plane.normal) >= 0)//moving away from each other
                continue;
            num++;
            X += xi;
        }
    }
    if (num == 0)//has no collision
        return;

    X /= num;
    Vector3 Rr = X - sharedResources.teapot.position;
    Vector3 linear = sharedResources.teapot.v;
    Vector3 angular = cross(sharedResources.teapot.w, Rr);
    Vector3 V = linear + angular;

    Vector3 project2Normal = dot(V, plane.normal) * plane.normal;
    Vector3 project2Tangent = V - project2Normal;
    float friction = glm::max(1 - mu * (1 + restitution) * length(project2Normal) / length(project2Tangent), 0.f);
    project2Normal = -restitution * project2Normal;
    project2Tangent = friction * project2Tangent;
    Vector3 Vnext = project2Normal + project2Tangent;

    Matrix4x4 K(1.0f / sharedResources.teapot.mass);
    Matrix4x4 inertia = orientation * sharedResources.teapot.inertiaTensor * glm::transpose(orientation);
    K -= glm::matrixCross4(Rr) * glm::inverse(inertia) * glm::matrixCross4(Rr);

    Vector3 impulse = glm::inverse(K) * Vector4(Vnext - V, 1.0);
    sharedResources.teapot.v += impulse / sharedResources.teapot.mass;
    sharedResources.teapot.w += Vector3(glm::inverse(inertia) * Vector4(cross(Rr, impulse), 1.0));

    restitution *= 0.9f;

    if (glm::length(sharedResources.teapot.v) < 0.01f || glm::length(sharedResources.teapot.w) < 0.01f)
    {
        restitution = 0;
    }
}