#pragma once
#include "renderGraph.h"
class ShadowRenderPass : public RenderPass
{
    SharedResources& sharedResources;
public:
    ShadowRenderPass(SharedResources& sharedResources);
    void SetupPipeline(PipelineState pipeline) override;
    void ResolveResources(ResolveState resolve) override;
    void OnRender(RenderPassState state) override;
};
