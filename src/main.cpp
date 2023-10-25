#include <filesystem>
#include <iostream>

#include "VulkanAbstractionLayer/Window.h"
#include "glm/gtx/matrix_cross_product.hpp"
#include "glm/gtx/quaternion.hpp"
#include "uniformPass.h"
#include "resLoader.h"
#include "camera.h"
using namespace VulkanAbstractionLayer;

void VulkanInfoCallback(const std::string& message)
{
    std::cout << "[INFO Vulkan]: " << message << std::endl;
}

void VulkanErrorCallback(const std::string& message)
{
    std::cout << "[ERROR Vulkan]: " << message << std::endl;
}

void WindowErrorCallback(const std::string& message)
{
    std::cerr << "[ERROR Window]: " << message << std::endl;
}

int main()
{
    if (std::filesystem::exists(APPLICATION_WORKING_DIRECTORY))
        std::filesystem::current_path(APPLICATION_WORKING_DIRECTORY);

    WindowCreateOptions windowOptions;
    windowOptions.Position = { 300.0f, 100.0f };
    windowOptions.Size = { 1280.0f, 720.0f };
    windowOptions.ErrorCallback = WindowErrorCallback;

    Window window(windowOptions);
    window.SetTitle("RigidBodySim");

    VulkanContextCreateOptions vulkanOptions;
    vulkanOptions.VulkanApiMajorVersion = 1;
    vulkanOptions.VulkanApiMinorVersion = 2;
    vulkanOptions.Extensions = window.GetRequiredExtensions();
    vulkanOptions.Layers = { "VK_LAYER_KHRONOS_validation" };
    vulkanOptions.ErrorCallback = VulkanErrorCallback;
    vulkanOptions.InfoCallback = VulkanInfoCallback;

    VulkanContext Vulkan(vulkanOptions);
    SetCurrentVulkanContext(Vulkan);

    ContextInitializeOptions deviceOptions;
    deviceOptions.PreferredDeviceType = DeviceType::DISCRETE_GPU;
    deviceOptions.ErrorCallback = VulkanErrorCallback;
    deviceOptions.InfoCallback = VulkanInfoCallback;

    Vulkan.InitializeContext(window.CreateWindowSurface(Vulkan), deviceOptions);

    constexpr size_t MaxMaterialCount = 256;
    SharedResources sharedResources{
        Buffer{ sizeof(UniformSubmitRenderPass::CameraUniform), BufferUsage::UNIFORM_BUFFER | BufferUsage::TRANSFER_DESTINATION, MemoryUsage::GPU_ONLY },
        Buffer{ sizeof(UniformSubmitRenderPass::ModelUniform),  BufferUsage::UNIFORM_BUFFER | BufferUsage::TRANSFER_DESTINATION, MemoryUsage::GPU_ONLY },
        Buffer{ sizeof(UniformSubmitRenderPass::LightUniform),  BufferUsage::UNIFORM_BUFFER | BufferUsage::TRANSFER_DESTINATION, MemoryUsage::GPU_ONLY },
        Buffer{ sizeof(MaterialData) * MaxMaterialCount,        BufferUsage::UNIFORM_BUFFER | BufferUsage::TRANSFER_DESTINATION, MemoryUsage::GPU_ONLY },
        { }, // meshes
        { }, // mesh textures
        { }, // materials
        { },//model
        { },//planes
        Image{ }, // brdf lut
        Image{ }, // skybox
        Image{ }, // skybox irradiance
    };

    LoadImage(sharedResources.BRDFLUT, "textures/brdf_lut.dds");
    LoadCubemap(sharedResources.Skybox, "textures/skybox.png");
    LoadCubemap(sharedResources.SkyboxIrradiance, "textures/skybox_irradiance.png");

    sharedResources.Meshes.push_back(CreatePlaneMesh(sharedResources.Materials, sharedResources.Textures));
    auto teapotModel = ModelLoader::LoadFromObj("models/teapot.obj");
    sharedResources.Meshes.push_back(CreateTeapotMesh(teapotModel, sharedResources.Materials, sharedResources.Textures));
	sharedResources.teapot.init(std::move(teapotModel));
	sharedResources.teapot.position = Vector3(-2, 20, 46);
	sharedResources.teapot.orientation = glm::quat(0.6,0.8f,0.0f,0.0f);
    PlaneGeometry plane0{};
    plane0.position = Vector3(0, 0.01f, 0);
    plane0.normal = Vector3(0, 1, 0);
    sharedResources.planes.push_back(plane0);

    std::unique_ptr<RenderGraph> renderGraph = CreateRenderGraph(sharedResources);

    Camera camera;
    Vector3 modelRotationPlane{ -HalfPi, Pi, 0.0f };
    Vector3 lightColor{ 0.7f, 0.7f, 0.7f };
    Vector3 lightDirection{ -0.3f, 1.0f, -0.6f };
    float lightBounds = 50.0f;
    float lightAmbientIntensity = 0.7f;

    window.OnResize([&Vulkan, &sharedResources, &renderGraph, &camera](Window& window, Vector2 size) mutable
    { 
        Vulkan.RecreateSwapchain((uint32_t)size.x, (uint32_t)size.y); 
        renderGraph = CreateRenderGraph(sharedResources);
        camera.AspectRatio = size.x / size.y;
    });
    
    ImGuiVulkanContext::Init(window, renderGraph->GetNodeByName("ImGuiPass").PassNative.RenderPassHandle);

    std::unordered_map<uint32_t, ImTextureID> imguiMappings;
    for (const auto& material : sharedResources.Materials)
    {
        if (imguiMappings.find(material.AlbedoTextureIndex) == imguiMappings.end())
        {
            imguiMappings.emplace(
                material.AlbedoTextureIndex,
                ImGuiVulkanContext::GetTextureId(sharedResources.Textures[material.AlbedoTextureIndex])
            );
        }
        if (imguiMappings.find(material.NormalTextureIndex) == imguiMappings.end())
        {
            imguiMappings.emplace(
                material.NormalTextureIndex,
                ImGuiVulkanContext::GetTextureId(sharedResources.Textures[material.NormalTextureIndex])
            );
        }
    }
    constexpr double fpsLimit = 1.0 / 60.0;
    double lastUpdateTime = 0;  
    double lastFrameTime = 0;
    while (!window.ShouldClose())
    {
        double now = window.GetTimeSinceCreation();
        double deltaTime = now - lastUpdateTime;
        window.PollEvents();

        if (Vulkan.IsRenderingEnabled() && (now - lastFrameTime) >= fpsLimit)
        {
            Vulkan.StartFrame();
            ImGuiVulkanContext::StartFrame();

            auto dt = ImGui::GetIO().DeltaTime;

            auto mouseMovement = ImGui::GetMouseDragDelta((ImGuiMouseButton)MouseButton::RIGHT, 0.0f);
            ImGui::ResetMouseDragDelta((ImGuiMouseButton)MouseButton::RIGHT);
            camera.Rotate(Vector2{ -mouseMovement.x, -mouseMovement.y } *dt);

            Vector3 movementDirection{ 0.0f };
        	if (ImGui::IsKeyDown((int)KeyCode::W))
                movementDirection += Vector3{ 1.0f,  0.0f,  0.0f };
            if (ImGui::IsKeyDown((int)KeyCode::A))
                movementDirection += Vector3{ 0.0f,  0.0f, -1.0f };
            if (ImGui::IsKeyDown((int)KeyCode::S))
                movementDirection += Vector3{ -1.0f,  0.0f,  0.0f };
            if (ImGui::IsKeyDown((int)KeyCode::D))
                movementDirection += Vector3{ 0.0f,  0.0f,  1.0f };
            if (ImGui::IsKeyDown((int)KeyCode::SPACE))
                movementDirection += Vector3{ 0.0f,  1.0f,  0.0f };
            if (ImGui::IsKeyDown((int)KeyCode::LEFT_SHIFT))
                movementDirection += Vector3{ 0.0f, -1.0f,  0.0f };
            if (movementDirection != Vector3{ 0.0f }) movementDirection = Normalize(movementDirection);
            camera.Move(movementDirection * dt);
            

            ImGui::Begin("Performace");
            ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
            ImGui::End();

            Vector3 low{ -lightBounds, -lightBounds, -lightBounds };
            Vector3 high{ lightBounds, lightBounds, lightBounds };

            auto& uniformSubmitPass = renderGraph->GetRenderPassByName<UniformSubmitRenderPass>("UniformSubmitPass");
            uniformSubmitPass.CameraUniform.Matrix = camera.GetMatrix();
            uniformSubmitPass.CameraUniform.Position = camera.Position;
            uniformSubmitPass.ModelUniform.Matrix[1]=glm::mat4_cast(sharedResources.teapot.orientation);
            uniformSubmitPass.ModelUniform.Matrix[1][3]=Vector4(sharedResources.teapot.position,1.0f);
            uniformSubmitPass.ModelUniform.Matrix[0] = MakeRotationMatrix(modelRotationPlane);
            uniformSubmitPass.LightUniform.Color = lightColor;
            uniformSubmitPass.LightUniform.AmbientIntensity = lightAmbientIntensity;
            uniformSubmitPass.LightUniform.Direction = Normalize(lightDirection);
            uniformSubmitPass.LightUniform.Projection =
                MakeOrthographicMatrix(low.x, high.x, low.y, high.y, low.z, high.z) *
                MakeLookAtMatrix(Vector3{ 0.0f, 0.0f, 0.0f }, -lightDirection, Vector3{ 0.001f, 1.0f, 0.001f });

            renderGraph->Execute(Vulkan.GetCurrentCommandBuffer());
            renderGraph->Present(Vulkan.GetCurrentCommandBuffer(), Vulkan.AcquireCurrentSwapchainImage(ImageUsage::TRANSFER_DISTINATION));

            ImGuiVulkanContext::EndFrame();
            Vulkan.EndFrame();

            lastFrameTime = now;
        }
        lastUpdateTime = now;

    }

    ImGuiVulkanContext::Destroy();

    return 0;
}
