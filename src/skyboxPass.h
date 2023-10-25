#pragma once
#include "renderGraph.h"
class SkyboxRenderPass : public RenderPass
{
    SharedResources& sharedResources;
    Sampler skyboxSampler;
public:
    SkyboxRenderPass(SharedResources& sharedResources);
    void SetupPipeline(PipelineState pipeline) override;
    void ResolveResources(ResolveState resolve) override;
    void OnRender(RenderPassState state) override;
};
