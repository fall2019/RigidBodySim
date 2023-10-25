#include "opaquePass.h"
#include "VulkanAbstractionLayer/GraphicShader.h"

OpaqueRenderPass::OpaqueRenderPass(SharedResources& sharedResources)
    : sharedResources(sharedResources)
{
    this->textureSampler.Init(Sampler::MinFilter::LINEAR, Sampler::MagFilter::LINEAR, Sampler::AddressMode::REPEAT, Sampler::MipFilter::LINEAR);
    this->depthSampler.Init(Sampler::MinFilter::NEAREST, Sampler::MagFilter::NEAREST, Sampler::AddressMode::CLAMP_TO_EDGE, Sampler::MipFilter::NEAREST);
}

void OpaqueRenderPass::SetupPipeline(PipelineState pipeline) 
{
    pipeline.Shader = std::make_unique<GraphicShader>(
        ShaderLoader::LoadFromSourceFile("src/main_vertex.glsl", ShaderType::VERTEX, ShaderLanguage::GLSL),
        ShaderLoader::LoadFromSourceFile("src/main_fragment.glsl", ShaderType::FRAGMENT, ShaderLanguage::GLSL)
    );

    pipeline.VertexBindings = {
        VertexBinding{
            VertexBinding::Rate::PER_VERTEX,
            5,
        },
        VertexBinding{
            VertexBinding::Rate::PER_INSTANCE,
            3,
        },
    };

    pipeline.DeclareAttachment("Output", Format::R8G8B8A8_UNORM);
    pipeline.DeclareAttachment("OutputDepth", Format::D32_SFLOAT_S8_UINT);

    pipeline.DescriptorBindings
        .Bind(0, "CameraUniformBuffer", UniformType::UNIFORM_BUFFER)
        .Bind(1, "MeshDataUniformBuffer", UniformType::UNIFORM_BUFFER)
        .Bind(2, "LightUniformBuffer", UniformType::UNIFORM_BUFFER)
        .Bind(3, "MaterialUniformBuffer", UniformType::UNIFORM_BUFFER)
        .Bind(4, this->textureSampler, UniformType::SAMPLER)
        .Bind(5, "Textures", UniformType::SAMPLED_IMAGE)
        .Bind(6, "ShadowDepth", this->depthSampler, UniformType::COMBINED_IMAGE_SAMPLER, ImageView::DEPTH_ONLY)
        .Bind(7, "BRDFLUT", this->textureSampler, UniformType::COMBINED_IMAGE_SAMPLER)
        .Bind(8, "Skybox", this->textureSampler, UniformType::COMBINED_IMAGE_SAMPLER)
        .Bind(9, "SkyboxIrradiance", this->textureSampler, UniformType::COMBINED_IMAGE_SAMPLER);

    pipeline.AddOutputAttachment("Output", ClearColor{ 0.5f, 0.8f, 1.0f, 1.0f });
    pipeline.AddOutputAttachment("OutputDepth", ClearDepthStencil{ });
}

void OpaqueRenderPass::ResolveResources(ResolveState resolve) 
{
    resolve.Resolve("Textures", this->sharedResources.Textures);
    resolve.Resolve("BRDFLUT", this->sharedResources.BRDFLUT);
    resolve.Resolve("Skybox", this->sharedResources.Skybox);
    resolve.Resolve("SkyboxIrradiance", this->sharedResources.SkyboxIrradiance);
}

void OpaqueRenderPass::OnRender(RenderPassState state) 
{
    auto& output = state.GetAttachment("Output");
    state.Commands.SetRenderArea(output);

    for (const auto& mesh : this->sharedResources.Meshes)
    {
        size_t indexCount = mesh.IndexBuffer.GetByteSize() / sizeof(ModelData::Index);
        size_t instanceCount = mesh.InstanceBuffer.GetByteSize() / sizeof(InstanceData);
        state.Commands.BindVertexBuffers(mesh.VertexBuffer, mesh.InstanceBuffer);
        state.Commands.BindIndexBufferUInt32(mesh.IndexBuffer);
        state.Commands.DrawIndexed((uint32_t)indexCount, (uint32_t)instanceCount);
    }
}