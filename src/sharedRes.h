#pragma once
#include "rigidbody.h"
struct Mesh
{
    Buffer VertexBuffer;
    Buffer IndexBuffer;
    Buffer InstanceBuffer;
};

struct InstanceData
{
    Vector3 Position;
    uint32_t MaterialIndex;
    uint32_t ModelIndex;
};

struct MaterialData
{
    uint32_t AlbedoTextureIndex;
    uint32_t NormalTextureIndex;
    float MetallicFactor;
    float RoughnessFactor;
};

struct SharedResources
{
    Buffer CameraUniformBuffer;
    Buffer MeshDataUniformBuffer;
    Buffer LightUniformBuffer;
    Buffer MaterialUniformBuffer;
    std::vector<Mesh> Meshes;
    std::vector<Image> Textures;
    std::vector<MaterialData> Materials;
    RigidBody teapot;
    std::vector<PlaneGeometry> planes;
    Image BRDFLUT;
    Image Skybox;
    Image SkyboxIrradiance;
};