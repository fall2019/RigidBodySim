#include "renderGraph.h"

#include "uniformPass.h"
#include "physicsPass.h"
#include "opaquePass.h"
#include "shadowPass.h"
#include "skyboxPass.h"
#include "VulkanAbstractionLayer/ImGuiRenderPass.h"
#include "VulkanAbstractionLayer/RenderGraphBuilder.h"
using namespace VulkanAbstractionLayer;

std::unique_ptr<RenderGraph> CreateRenderGraph(SharedResources& resources)
{
    RenderGraphBuilder renderGraphBuilder;
    renderGraphBuilder
        .AddRenderPass("UniformSubmitPass", std::make_unique<UniformSubmitRenderPass>(resources))
        .AddRenderPass("PhysicsPass", std::make_unique<RigidBodyDynamicsPass>(resources))
        .AddRenderPass("ShadowPass", std::make_unique<ShadowRenderPass>(resources))
        .AddRenderPass("OpaquePass", std::make_unique<OpaqueRenderPass>(resources))
        .AddRenderPass("SkyboxPass", std::make_unique<SkyboxRenderPass>(resources))
        .AddRenderPass("ImGuiPass", std::make_unique<ImGuiRenderPass>("Output"))
        .SetOutputName("Output");

    return renderGraphBuilder.Build();
}