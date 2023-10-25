#include"shadowPass.h"
#include "VulkanAbstractionLayer/GraphicShader.h"
using namespace VulkanAbstractionLayer;
ShadowRenderPass::ShadowRenderPass(SharedResources& sharedResources)
    : sharedResources(sharedResources)
{

}
void ShadowRenderPass::SetupPipeline(PipelineState pipeline) 
{
    pipeline.Shader = std::make_unique<GraphicShader>(
        ShaderLoader::LoadFromSourceFile("src/shadow_vertex.glsl", ShaderType::VERTEX, ShaderLanguage::GLSL),
        ShaderLoader::LoadFromSourceFile("src/shadow_fragment.glsl", ShaderType::FRAGMENT, ShaderLanguage::GLSL)
    );

    pipeline.DeclareAttachment("ShadowDepth", Format::D32_SFLOAT_S8_UINT, 2048, 2048);

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

    pipeline.DescriptorBindings
        .Bind(1, "MeshDataUniformBuffer", UniformType::UNIFORM_BUFFER)
        .Bind(2, "LightUniformBuffer", UniformType::UNIFORM_BUFFER);

    pipeline.AddOutputAttachment("ShadowDepth", ClearDepthStencil{ });
}

void ShadowRenderPass::ResolveResources(ResolveState resolve) 
{

}

void ShadowRenderPass::OnRender(RenderPassState state) 
{
    auto& output = state.GetAttachment("ShadowDepth");
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