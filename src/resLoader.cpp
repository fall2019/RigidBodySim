#include "VulkanAbstractionLayer/VulkanContext.h"
#include "sharedRes.h"
#include "resLoader.h"

void LoadCubemap(Image& image, const std::string& filepath)
{
    auto& vulkanContext = GetCurrentVulkanContext();
    auto& stageBuffer = vulkanContext.GetCurrentStageBuffer();
    auto& commandBuffer = vulkanContext.GetCurrentCommandBuffer();

    commandBuffer.Begin();

    auto cubemapData = ImageLoader::LoadCubemapImageFromFile(filepath);
    image.Init(
        cubemapData.FaceWidth,
        cubemapData.FaceHeight,
        cubemapData.FaceFormat,
        ImageUsage::TRANSFER_DISTINATION | ImageUsage::TRANSFER_SOURCE | ImageUsage::SHADER_READ,
        MemoryUsage::GPU_ONLY,
        ImageOptions::CUBEMAP | ImageOptions::MIPMAPS
    );

    for (uint32_t layer = 0; layer < cubemapData.Faces.size(); layer++)
    {
        const auto& face = cubemapData.Faces[layer];
        auto textureAllocation = stageBuffer.Submit(face.data(), face.size());

        commandBuffer.CopyBufferToImage(
            BufferInfo{ stageBuffer.GetBuffer(), textureAllocation.Offset },
            ImageInfo{ image, ImageUsage::UNKNOWN, 0, layer }
        );
    }

    commandBuffer.GenerateMipLevels(image, ImageUsage::TRANSFER_DISTINATION, BlitFilter::LINEAR);

    commandBuffer.TransferLayout(image, ImageUsage::TRANSFER_DISTINATION, ImageUsage::SHADER_READ);

    stageBuffer.Flush();
    commandBuffer.End();

    vulkanContext.SubmitCommandsImmediate(commandBuffer);
    stageBuffer.Reset();
}

void LoadImage(Image& image, const std::string& filepath)
{
    auto& vulkanContext = GetCurrentVulkanContext();
    auto& stageBuffer = vulkanContext.GetCurrentStageBuffer();
    auto& commandBuffer = vulkanContext.GetCurrentCommandBuffer();

    commandBuffer.Begin();

    auto imageData = ImageLoader::LoadImageFromFile(filepath);
    image.Init(
        imageData.Width,
        imageData.Height,
        imageData.ImageFormat,
        ImageUsage::TRANSFER_DISTINATION | ImageUsage::SHADER_READ,
        MemoryUsage::GPU_ONLY,
        ImageOptions::DEFAULT
    );

    auto textureAllocation = stageBuffer.Submit(imageData.ByteData.data(), imageData.ByteData.size());

    commandBuffer.CopyBufferToImage(
        BufferInfo{ stageBuffer.GetBuffer(), textureAllocation.Offset },
        ImageInfo{ image, ImageUsage::UNKNOWN, 0, 0 }
    );

    commandBuffer.TransferLayout(image, ImageUsage::TRANSFER_DISTINATION, ImageUsage::SHADER_READ);

    stageBuffer.Flush();
    commandBuffer.End();

    vulkanContext.SubmitCommandsImmediate(commandBuffer);
    stageBuffer.Reset();
}

Mesh CreateMesh(ArrayView<ModelData::Vertex> vertices, ArrayView<ModelData::Index> indices, ArrayView<InstanceData> instances, ArrayView<ImageData> textures, std::vector<Image>& images)
{
    Mesh result;

    auto& vulkanContext = GetCurrentVulkanContext();
    auto& stageBuffer = vulkanContext.GetCurrentStageBuffer();
    auto& commandBuffer = vulkanContext.GetCurrentCommandBuffer();

    commandBuffer.Begin();

    auto instanceAllocation = stageBuffer.Submit(MakeView(instances));
    auto indexAllocation = stageBuffer.Submit(MakeView(indices));
    auto vertexAllocation = stageBuffer.Submit(MakeView(vertices));

    result.InstanceBuffer.Init(instanceAllocation.Size, BufferUsage::VERTEX_BUFFER | BufferUsage::TRANSFER_DESTINATION, MemoryUsage::GPU_ONLY);
    result.IndexBuffer.Init(indexAllocation.Size, BufferUsage::INDEX_BUFFER | BufferUsage::TRANSFER_DESTINATION, MemoryUsage::GPU_ONLY);
    result.VertexBuffer.Init(vertexAllocation.Size, BufferUsage::VERTEX_BUFFER | BufferUsage::TRANSFER_DESTINATION, MemoryUsage::GPU_ONLY);

    commandBuffer.CopyBuffer(
        BufferInfo{ stageBuffer.GetBuffer(), instanceAllocation.Offset },
        BufferInfo{ result.InstanceBuffer, 0 },
        instanceAllocation.Size
    );
    commandBuffer.CopyBuffer(
        BufferInfo{ stageBuffer.GetBuffer(), indexAllocation.Offset },
        BufferInfo{ result.IndexBuffer, 0 },
        indexAllocation.Size
    );
    commandBuffer.CopyBuffer(
        BufferInfo{ stageBuffer.GetBuffer(), vertexAllocation.Offset },
        BufferInfo{ result.VertexBuffer, 0 },
        vertexAllocation.Size
    );

    for (const auto& texture : textures)
    {
        auto& image = images.emplace_back();
        image.Init(
            texture.Width,
            texture.Height,
            texture.ImageFormat,
            ImageUsage::TRANSFER_DISTINATION | ImageUsage::TRANSFER_SOURCE | ImageUsage::SHADER_READ,
            MemoryUsage::GPU_ONLY,
            ImageOptions::MIPMAPS
        );

        auto textureAllocation = stageBuffer.Submit(texture.ByteData.data(), texture.ByteData.size());

        commandBuffer.CopyBufferToImage(
            BufferInfo{ stageBuffer.GetBuffer(), textureAllocation.Offset },
            ImageInfo{ image, ImageUsage::UNKNOWN, 0, 0 }
        );
        commandBuffer.GenerateMipLevels(image, ImageUsage::TRANSFER_DISTINATION, BlitFilter::LINEAR);

        commandBuffer.TransferLayout(image, ImageUsage::TRANSFER_DISTINATION, ImageUsage::SHADER_READ);
    }

    stageBuffer.Flush();
    commandBuffer.End();

    vulkanContext.SubmitCommandsImmediate(commandBuffer);
    stageBuffer.Reset();

    return result;
}

Mesh CreatePlaneMesh(std::vector<MaterialData>& globalMaterials, std::vector<Image>& globalImages)
{
    std::vector vertices = {
        ModelData::Vertex{ { -500.0f, -500.0f, -0.01f }, { -15.0f, -15.0f }, { 0.0f, 0.0f, 1.0f } },
        ModelData::Vertex{ {  500.0f,  500.0f, -0.01f }, {  15.0f,  15.0f }, { 0.0f, 0.0f, 1.0f } },
        ModelData::Vertex{ { -500.0f,  500.0f, -0.01f }, { -15.0f,  15.0f }, { 0.0f, 0.0f, 1.0f } },
        ModelData::Vertex{ {  500.0f,  500.0f, -0.01f }, {  15.0f,  15.0f }, { 0.0f, 0.0f, 1.0f } },
        ModelData::Vertex{ { -500.0f, -500.0f, -0.01f }, { -15.0f, -15.0f }, { 0.0f, 0.0f, 1.0f } },
        ModelData::Vertex{ {  500.0f, -500.0f, -0.01f }, {  15.0f, -15.0f }, { 0.0f, 0.0f, 1.0f } },
    };
    std::vector<ModelData::Index> indices = {
        0, 1, 2, 3, 4, 5
    };
    std::vector instances = {
        InstanceData{ { 0.0f, 0.0f, 0.0f }, (uint32_t)globalMaterials.size(), 0},
    };

    globalMaterials.push_back(MaterialData{
        (uint32_t)globalImages.size() + 0,
        (uint32_t)globalImages.size() + 1,
        0.0f, // metallic
        0.9f, // roughness
        });

    auto albedoTexture = ImageLoader::LoadImageFromFile("textures/brickwall.jpg");
    auto normalTexture = ImageLoader::LoadImageFromFile("textures/brickwall_normal.jpg");

    return CreateMesh(vertices, indices, instances, MakeView(std::array{ albedoTexture, normalTexture }), globalImages);
}

Mesh CreateTeapotMesh(ModelData& model, std::vector<MaterialData>& globalMaterials, std::vector<Image>& globalImages)
{
    std::vector instances = {
        InstanceData{ { 0.0f, 0.0f, -40.0f }, (uint32_t)globalMaterials.size() + 0, 1 }
    };

    auto& vertices = model.Shapes.front().Vertices;
    auto& indices = model.Shapes.front().Indices;

    std::array albedoTextures = {
        std::vector<uint8_t>{ 150, 225, 100, 255 },
    };

    std::array normalTextures = {
        std::vector < uint8_t>{ 127, 127, 255, 255 }
    };

    std::array textures = {
        ImageData{ std::move(normalTextures[0]), Format::R8G8B8A8_UNORM, 1, 1 },
        ImageData{ std::move(albedoTextures[0]), Format::R8G8B8A8_UNORM, 1, 1 },
    };

    globalMaterials.push_back(MaterialData{
        (uint32_t)globalImages.size() + 1,
        (uint32_t)globalImages.size(), // default normal
        0.0f, // metallic
        1.0f, // roughness
        });
    return CreateMesh(vertices, indices, instances, textures, globalImages);
}
