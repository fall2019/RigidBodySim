#pragma once
#include "renderGraph.h"
using namespace VulkanAbstractionLayer;
class RigidBodyDynamicsPass : public RenderPass
{
    SharedResources& sharedResources;
    Vector3 inputLinearVelocity = Vector3(0, 10, 4);
    Vector3 inputAngularVelocity = Vector3(1, 0, 1);
    Vector3 inputPosition = Vector3(-2, 70, 46);
    Vector3 inputEulerAngle = Vector3(0, 0, 0);

    static inline constexpr float dt = 0.1f;
    static inline constexpr float linearDecay = 0.999f;    // for velocity decay
    static inline constexpr float angularDecay = 0.98f;
    static inline constexpr Vector3 gravity = Vector3(0, -9.8f, 0);
    static inline constexpr float mu = 0.5f;
    float restitution = 0.8f;	    // for collision
public:
    RigidBodyDynamicsPass(SharedResources& sharedResources);
    void SetupPipeline(PipelineState pipeline) override;
    void ResolveResources(ResolveState resolve) override;
    void BeforeRender(RenderPassState state) override;
    void OnRender(RenderPassState state) override;
    void UpdateVelocity();
    Vector3 UpdatePosition(Vector3 x) const;
    glm::quat UpdateRotation(glm::quat q);
    void UpdateAngular();
    bool CollideWithPlane(const PlaneGeometry& plane, Vector3 x) const;
    void SolveCollision(const PlaneGeometry& plane);
};
