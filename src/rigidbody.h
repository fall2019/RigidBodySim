#pragma once
#include "VulkanAbstractionLayer/VectorMath.h"
#include "VulkanAbstractionLayer/ModelLoader.h"
using namespace VulkanAbstractionLayer;
struct RigidBody
{
    Vector3 v{ 0.0f,0.0f,0.0f };
    Vector3 w{ 0.0f,0.0f,0.0f };
    float mass{ 0.0f };
    Matrix4x4 inertiaTensor;
    Vector3 position;
    glm::quat orientation;
    ModelData modelData;
    void init(ModelData&& model)
    {
        constexpr float m = 1;
        for (auto& shape : model.Shapes)
        {
            for (auto& vert : shape.Vertices)
            {
                auto& pos = vert.Position;
                mass += m;
                float diag = m * dot(pos, pos);
                inertiaTensor[0][0] += diag;
                inertiaTensor[1][1] += diag;
                inertiaTensor[2][2] += diag;
                inertiaTensor[0][0] -= m * pos[0] * pos[0];
                inertiaTensor[1][0] -= m * pos[0] * pos[1];
                inertiaTensor[2][0] -= m * pos[0] * pos[2];
                inertiaTensor[0][1] -= m * pos[1] * pos[0];
                inertiaTensor[1][1] -= m * pos[1] * pos[1];
                inertiaTensor[2][1] -= m * pos[1] * pos[2];
                inertiaTensor[0][2] -= m * pos[2] * pos[0];
                inertiaTensor[1][2] -= m * pos[2] * pos[1];
                inertiaTensor[2][2] -= m * pos[2] * pos[2];
            }
        }
        inertiaTensor[3][3] = 1;
        modelData = std::move(model);
    }
};
struct PlaneGeometry
{
    Vector3 position;
    Vector3 normal;
};
