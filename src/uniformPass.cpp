#include "uniformPass.h"

UniformSubmitRenderPass::UniformSubmitRenderPass(SharedResources& sharedResources)
    : sharedResources(sharedResources)
{

}

void UniformSubmitRenderPass::SetupPipeline(PipelineState pipeline) 
{
    pipeline.AddDependency("CameraUniformBuffer", BufferUsage::TRANSFER_DESTINATION);
    pipeline.AddDependency("MeshDataUniformBuffer", BufferUsage::TRANSFER_DESTINATION);
    pipeline.AddDependency("LightUniformBuffer", BufferUsage::TRANSFER_DESTINATION);
    pipeline.AddDependency("MaterialUniformBuffer", BufferUsage::TRANSFER_DESTINATION);
}

void UniformSubmitRenderPass::ResolveResources(ResolveState resolve) 
{
    resolve.Resolve("CameraUniformBuffer", this->sharedResources.CameraUniformBuffer);
    resolve.Resolve("MeshDataUniformBuffer", this->sharedResources.MeshDataUniformBuffer);
    resolve.Resolve("LightUniformBuffer", this->sharedResources.LightUniformBuffer);
    resolve.Resolve("MaterialUniformBuffer", this->sharedResources.MaterialUniformBuffer);
}

void UniformSubmitRenderPass::OnRender(RenderPassState state) 
{
    auto FillUniform = [&state](const auto& uniformData, const auto& uniformBuffer) mutable
    {
        auto& stageBuffer = GetCurrentVulkanContext().GetCurrentStageBuffer();
        auto uniformAllocation = stageBuffer.Submit(&uniformData);
        state.Commands.CopyBuffer(
            BufferInfo{ stageBuffer.GetBuffer(), uniformAllocation.Offset },
            BufferInfo{ uniformBuffer, 0 },
            uniformAllocation.Size
        );
    };

    auto FillUniformArray = [&state](const auto& uniformData, const auto& uniformBuffer) mutable
    {
        auto& stageBuffer = GetCurrentVulkanContext().GetCurrentStageBuffer();
        auto uniformAllocation = stageBuffer.Submit(MakeView(uniformData));
        state.Commands.CopyBuffer(
            BufferInfo{ stageBuffer.GetBuffer(), uniformAllocation.Offset },
            BufferInfo{ uniformBuffer, 0 },
            uniformAllocation.Size
        );
    };

    FillUniform(this->CameraUniform, this->sharedResources.CameraUniformBuffer);
    FillUniform(this->ModelUniform, this->sharedResources.MeshDataUniformBuffer);
    FillUniform(this->LightUniform, this->sharedResources.LightUniformBuffer);
    FillUniformArray(this->sharedResources.Materials, this->sharedResources.MaterialUniformBuffer);
}