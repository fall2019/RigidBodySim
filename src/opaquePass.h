#pragma once
#include "renderGraph.h"
class OpaqueRenderPass : public RenderPass
{
    SharedResources& sharedResources;
    Sampler textureSampler;
    Sampler depthSampler;
public:
    OpaqueRenderPass(SharedResources& sharedResources);
    virtual void SetupPipeline(PipelineState pipeline) override;
    virtual void ResolveResources(ResolveState resolve) override;
    virtual void OnRender(RenderPassState state) override;
};
