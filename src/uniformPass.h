#pragma once
#include "renderGraph.h"
using namespace VulkanAbstractionLayer;
class UniformSubmitRenderPass : public RenderPass
{
    SharedResources& sharedResources;

public:
    struct CameraUniformData
    {
        Matrix4x4 Matrix;
        Vector3 Position;
    } CameraUniform;

    struct ModelUniformData
    {
        Matrix4x4 Matrix[2];
    } ModelUniform;

    struct LightUniformData
    {
        Matrix4x4 Projection;
        Vector3 Color;
        float AmbientIntensity;
        Vector3 Direction;
    } LightUniform;

    UniformSubmitRenderPass(SharedResources& sharedResources);
	void SetupPipeline(PipelineState pipeline) override;
    void ResolveResources(ResolveState resolve) override;
    void OnRender(RenderPassState state) override;
};
