#pragma once
#include "VulkanAbstractionLayer/ArrayUtils.h"
#include "VulkanAbstractionLayer/Image.h"
#include "VulkanAbstractionLayer/ModelLoader.h"
#include "VulkanAbstractionLayer/ImageLoader.h"
using namespace VulkanAbstractionLayer;
void LoadCubemap(Image& image, const std::string& filepath);
void LoadImage(Image& image, const std::string& filepath);
Mesh CreateMesh(ArrayView<ModelData::Vertex> vertices, ArrayView<ModelData::Index> indices, ArrayView<InstanceData> instances, ArrayView<ImageData> textures, std::vector<Image>& images);
Mesh CreatePlaneMesh(std::vector<MaterialData>& globalMaterials, std::vector<Image>& globalImages);
Mesh CreateTeapotMesh(ModelData& model, std::vector<MaterialData>& globalMaterials, std::vector<Image>& globalImages);