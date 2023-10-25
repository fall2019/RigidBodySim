#include "skyboxPass.h"

#include "VulkanAbstractionLayer/GraphicShader.h"

SkyboxRenderPass::SkyboxRenderPass(SharedResources& sharedResources)
    : sharedResources(sharedResources)
{
    skyboxSampler.Init(Sampler::MinFilter::LINEAR, Sampler::MagFilter::LINEAR, Sampler::AddressMode::CLAMP_TO_EDGE, Sampler::MipFilter::LINEAR);
}

void SkyboxRenderPass::SetupPipeline(PipelineState pipeline) 
{
    pipeline.Shader = std::make_unique<GraphicShader>(
        ShaderLoader::LoadFromSourceFile("src/skybox_vertex.glsl", ShaderType::VERTEX, ShaderLanguage::GLSL),
        ShaderLoader::LoadFromSourceFile("src/skybox_fragment.glsl", ShaderType::FRAGMENT, ShaderLanguage::GLSL)
    );

    pipeline.DescriptorBindings
        .Bind(0, "CameraUniformBuffer", UniformType::UNIFORM_BUFFER)
        .Bind(8, "Skybox", skyboxSampler, UniformType::COMBINED_IMAGE_SAMPLER);

    pipeline.AddOutputAttachment("Output", AttachmentState::LOAD_COLOR);
    pipeline.AddOutputAttachment("OutputDepth", AttachmentState::LOAD_DEPTH_SPENCIL);
}

void SkyboxRenderPass::ResolveResources(ResolveState resolve) 
{

}

void SkyboxRenderPass::OnRender(RenderPassState state) 
{
    auto& output = state.GetAttachment("Output");
    state.Commands.SetRenderArea(output);

    constexpr uint32_t SkyboxVertexCount = 36;
    state.Commands.Draw(SkyboxVertexCount, 1);
}