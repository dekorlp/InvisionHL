#include "Mesh.h"
#include "Light.h"

#include "GraphicsInstance.h"


GraphicsInstance::GraphicsInstance()
{
	log.Open("Log.txt");
	Invision::Log::SetLogger(&log);
}

void GraphicsInstance::Init(HWND hwnd, int width, int height)
{
	try
	{
		mHwnd = hwnd;

#ifdef NDEBUG
		graphicsEngine = Invision::Create_engine(Invision::EngineType::Vulkan);
#else
		graphicsEngine = Invision::Create_engine(Invision::EngineType::Vulkan, log.GetOutputStream());
#endif
		graphicsEngine->Init();


		Invision::CanvasDimensions dim = { mHwnd, width, height };

		
		graphicsInstance = graphicsEngine->CreateInstance(dim, renderPass, framebuffer, commandBuffer);

		renderer = graphicsInstance->CreateRenderer();

		mGenUniformBuffer = graphicsInstance->CreateUniformBuffer();
		mGenUniformBuffer->CreateUniformBinding(0, 0, 1, Invision::SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject))
			.CreateUniformBinding(0, 1, 1, Invision::SHADER_STAGE_VERTEX_BIT | Invision::SHADER_STAGE_FRAGMENT_BIT, sizeof(GeneralUbo) )
			.CreateUniformBinding(0, 2, 1, Invision::SHADER_STAGE_VERTEX_BIT | Invision::SHADER_STAGE_FRAGMENT_BIT, sizeof(LightUbo)).CreateUniformBuffer();

		mGeometryUniformBuffer = graphicsInstance->CreateUniformBuffer();
		mGeometryUniformBuffer->CreateUniformBinding(0, 0, 1, Invision::SHADER_STAGE_VERTEX_BIT | Invision::SHADER_STAGE_GEOMETRY_BIT, sizeof(UniformBufferObject))
			.CreateUniformBinding(0, 1, 1, Invision::SHADER_STAGE_VERTEX_BIT | Invision::SHADER_STAGE_GEOMETRY_BIT | Invision::SHADER_STAGE_FRAGMENT_BIT, sizeof(GeneralUbo)).CreateUniformBuffer();

	}
	catch (std::runtime_error& err) {
		std::stringstream ss;
		ss << "Error encountered trying to create the Vulkan canvas:\n";
		ss << err.what();
		INVISION_LOG_ERROR(ss.str());
		return;
	}
	catch (Invision::InvisionException& iEx)
	{
		std::stringstream ss;
		ss << "Error encountered trying to create the Vulkan canvas:\n";
		ss << iEx.what();
		// TODO OutputDebug Replace with own MessageBox
		//wxMessageBox( ss.str(), "Info");
		INVISION_LOG_ERROR(ss.str());
		return;
	}
	catch (Invision::InvisionBaseRendererException& iEx)
	{
		std::stringstream ss;
		ss << "Vulkan Error encountered trying to create the Vulkan canvas:\n";
		ss << iEx.what();
		INVISION_LOG_ERROR(ss.str());
		return;
	}
}

void GraphicsInstance::RecreateSwapchain(const int width, const int height)
{
	// setup swapchain
	graphicsInstance->ResetPresentation({ mHwnd, width, height }, renderPass, framebuffer, commandBuffer);
}

bool GraphicsInstance::PrepareFrame(const int width, const int height)
{
	bool recreateSwapchain = renderer->PrepareFrame();

	return recreateSwapchain;
}

void GraphicsInstance::Render()
{
	mGenUniformBuffer->UpdateUniform(&mGUbo, sizeof(mGUbo), 0, 1);
	mGenUniformBuffer->UpdateUniform(&mLightUbo, sizeof(mLightUbo), 0, 2);

	mGeometryUniformBuffer->UpdateUniform(&mGUbo, sizeof(mGUbo), 0, 1);

	renderer->Draw(commandBuffer);
}

bool GraphicsInstance::SubmitFrame(const int width, const int height)
{
	bool recreateSwapchainIsNecessary = renderer->SubmitFrame();


	if (recreateSwapchainIsNecessary) RecreateSwapchain(width, height);

	return recreateSwapchainIsNecessary;
}

void GraphicsInstance::SetProjectionViewMatrix(glm::mat4 view, glm::mat4 proj, glm::vec3 cameraPosition)
{
	mGUbo.proj = proj;
	mGUbo.view = view;
	mGUbo.viewPos = cameraPosition;

	// light for testing shader dev
	//mLightUbo.light[0].position = glm::vec4(0.0f, -4.0f, 0.0f, 0.0f);
	//mLightUbo.light[0].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	//mLightUbo.countLights = 1;

}

LightIndex GraphicsInstance::AddLight(Light& light)
{
	mLightUbo.countLights += 1;
	Light lgtTemp = light;


	mLightUbo.light[mLightUbo.countLights - 1] = (light.GetLightInformations());

	return (LightIndex)(mLightUbo.countLights - 1);
}

void GraphicsInstance::UpdateLight(Light& light, LightIndex lightIndex)
{
	mLightUbo.light[lightIndex] = light.GetLightInformations();
}

void GraphicsInstance::BeginCommandBuffer(const int width, const int height)
{
	commandBuffer->BeginCommandBuffer().
		SetViewport({ 0, 0, (float)width, (float)height, 0.0, 1.0 }).
		SetScissor({ 0, 0, (uint32_t)width, (uint32_t)height }).
		BeginRenderPass(renderPass, framebuffer, 0, 0, width, height);
	
}

void GraphicsInstance::EndCommandBuffer()
{
	commandBuffer->EndRenderPass();
	commandBuffer->EndCommandBuffer();
}

void GraphicsInstance::Destroy()
{
	
}

GraphicsInstance::~GraphicsInstance()
{
	log.Close();
}


// child class
void DrawingInstance::BindMesh(Mesh &mesh)
{
	std::shared_ptr <Invision::ICommandBuffer> commandBuffer = mParentAddr->GetCommandBuffer();

	if (mesh.IsIndexed())
	{
		commandBuffer->BindPipeline(mesh.GetPipeline()).
			BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1).
			BindDescriptorSets(GetGeneralUniformBufferObject(), mesh.GetPipeline()).
			BindIndexBuffer(mesh.GetIndexBuffer(), Invision::INDEX_TYPE_UINT32).
			//Draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0).
			DrawIndexed(static_cast<uint32_t>(mesh.GetIndizes().size()), 1, 0, 0, 0);

#ifdef INVISION_HL_DRAW_NORMALS

		commandBuffer->BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1);
		commandBuffer->BindPipeline(mesh.GetGeomPipeline());
		commandBuffer->BindDescriptorSets(GetGeometryUniformBufferObject(), mesh.GetGeomPipeline());
		commandBuffer->DrawIndexed(static_cast<uint32_t>(mesh.GetIndizes().size()), 1, 0, 0, 0);
#endif
	}
	else
	{
		commandBuffer->BindPipeline(mesh.GetPipeline()).
			BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1).
			BindDescriptorSets(GetGeneralUniformBufferObject(), mesh.GetPipeline()).
			//Draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0).
			Draw(static_cast<uint32_t>(mesh.GetVertizes().size()), 1, 0);

#ifdef INVISION_HL_DRAW_NORMALS

		commandBuffer->BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1);
		commandBuffer->BindPipeline(mesh.GetGeomPipeline());
		commandBuffer->BindDescriptorSets(GetGeometryUniformBufferObject(), mesh.GetGeomPipeline());
		commandBuffer->Draw(static_cast<uint32_t>(mesh.GetVertizes().size()), 1, 0);
#endif
	}

	
}