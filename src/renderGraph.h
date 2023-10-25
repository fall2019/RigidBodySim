#pragma once
#include "VulkanAbstractionLayer/VulkanContext.h"
#include "VulkanAbstractionLayer/ImGuiContext.h"
#include "VulkanAbstractionLayer/RenderPass.h"
#include "VulkanAbstractionLayer/RenderGraph.h"
#include "sharedRes.h"
std::unique_ptr<RenderGraph> CreateRenderGraph(SharedResources& resources);