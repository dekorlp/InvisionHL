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

		// gPass Initialization
		mGBuffer.gRenderPass = graphicsInstance->CreateRenderPass();
		mGBuffer.positionsAttachment = graphicsInstance->CreateColorAttachment(FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE, Invision::FORMAT_R16G16B16A16_SFLOAT);
		mGBuffer.albedoAttachment = graphicsInstance->CreateColorAttachment(FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE, Invision::FORMAT_R16G16B16A16_SFLOAT);
		mGBuffer.normalAttachment = graphicsInstance->CreateColorAttachment(FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE, Invision::FORMAT_R16G16B16A16_SFLOAT);
		mGBuffer.depthAttachment = graphicsInstance->CreateDepthAttachment(FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE);

		// gPass Sampler Settings
		mGBuffer.positionsAttachment->CreateTextureSampler(Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP);
		mGBuffer.albedoAttachment->CreateTextureSampler(Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP);
		mGBuffer.normalAttachment->CreateTextureSampler(Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP);
		mGBuffer.depthAttachment->CreateTextureSampler(Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP);

		// gPass to RenderPass
		mGBuffer.gRenderPass->AddAttachment(Invision::ATTACHMENT_TYPE_COLOR, mGBuffer.positionsAttachment); // world Space Positions
		mGBuffer.gRenderPass->AddAttachment(Invision::ATTACHMENT_TYPE_COLOR, mGBuffer.normalAttachment); // Normals
		mGBuffer.gRenderPass->AddAttachment(Invision::ATTACHMENT_TYPE_COLOR, mGBuffer.albedoAttachment); // Albedo
		mGBuffer.gRenderPass->AddAttachment(Invision::ATTACHMENT_TYPE_DEPTH, mGBuffer.depthAttachment); // Depth
		mGBuffer.gRenderPass->CreateRenderPass();

		mGBuffer.gFramebuffer = graphicsInstance->CreateFramebuffer(mGBuffer.gRenderPass, FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE);
		mGBuffer.gCommandbuffer = graphicsInstance->CreateCommandBuffer(mGBuffer.gFramebuffer);

		std::shared_ptr<Invision::IVertexBindingDescription> bindingDescr = graphicsInstance->CreateVertexBindingDescription();
		bindingDescr->CreateVertexBinding(0, sizeof(Vertex), Invision::VERTEX_INPUT_RATE_VERTEX)
			->CreateAttribute(0, Invision::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position))
			.CreateAttribute(1, Invision::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
			.CreateAttribute(2, Invision::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));

		

		//mSBuffer.sDepthAttachment = graphicsInstance->CreateColorAttachment(FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE, Invision::FORMAT_R16G16B16A16_SFLOAT);
		mSBuffer.sDepthAttachment = graphicsInstance->CreateDepthAttachment(FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE, true);

		mSBuffer.sDepthAttachment->CreateTextureSampler(Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_FILTER_MODE_NEAREST, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP, Invision::SAMPLER_ADDRESS_MODE_CLAMP);
		mSBuffer.sRenderPass = graphicsInstance->CreateDepthOnlyRenderPass(mSBuffer.sDepthAttachment);

		mSBuffer.sRenderPass->CreateRenderPass();
		mSBuffer.sFramebuffer = graphicsInstance->CreateFramebuffer(mSBuffer.sRenderPass, FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE);
		mSBuffer.sCommandbuffer = graphicsInstance->CreateCommandBuffer(mSBuffer.sFramebuffer);
		

		// Deferred Shading Initialization
		pipeline = graphicsInstance->CreatePipeline(&Invision::PipelineProperties(Invision::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, Invision::POLYGON_MODE_FILL, Invision::CULL_MODE_FRONT_BIT, Invision::FRONT_FACE_COUNTER_CLOCKWISE, 1.0f));

		DeferredUniformBuffer = graphicsInstance->CreateUniformBuffer();
		DeferredUniformBuffer->CreateImageBinding(0, 0, 1, Invision::SHADER_STAGE_FRAGMENT_BIT, mGBuffer.albedoAttachment)
			.CreateImageBinding(0, 1, 1, Invision::SHADER_STAGE_FRAGMENT_BIT, mGBuffer.normalAttachment)
			.CreateImageBinding(0, 2, 1, Invision::SHADER_STAGE_FRAGMENT_BIT, mGBuffer.positionsAttachment)
			.CreateImageBinding(0, 3, 1, Invision::SHADER_STAGE_FRAGMENT_BIT, mSBuffer.sDepthAttachment)
			.CreateUniformBinding(0, 4, 1, Invision::SHADER_STAGE_FRAGMENT_BIT, sizeof(UniformOptionsBuffer))
			.CreateUniformBinding(0, 5, 1, Invision::SHADER_STAGE_FRAGMENT_BIT, sizeof(LightUbo) +(sizeof(SLight) *8))
			.CreateUniformBinding(0, 6, 1, Invision::SHADER_STAGE_FRAGMENT_BIT, sizeof(GeneralUbo)).CreateUniformBuffer();

		auto deferredVertShaderCode = readFile(std::string("C:/Repository/InvisionHL/HL/HL/Shader/deferred.vert.spv"));
		auto deferredFragShaderCode = readFile(std::string("C:/Repository/InvisionHL/HL/HL/Shader/deferred.frag.spv"));
		pipeline->AddShader(deferredVertShaderCode, Invision::SHADER_STAGE_VERTEX_BIT);
		pipeline->AddShader(deferredFragShaderCode, Invision::SHADER_STAGE_FRAGMENT_BIT);
		pipeline->AddUniformBuffer(DeferredUniformBuffer);
		pipeline->CreatePipeline(renderPass);

		renderer = graphicsInstance->CreateRenderer();

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

	// Reset GBuffer (Framebuffer and Commandbuffer)
	mGBuffer.gCommandbuffer.reset();
	mGBuffer.gCommandbuffer = graphicsInstance->CreateCommandBuffer(mGBuffer.gFramebuffer);

	// Reset SBuffer (Shadowbuffer)
	mSBuffer.sCommandbuffer.reset();
	mSBuffer.sCommandbuffer = graphicsInstance->CreateCommandBuffer(mSBuffer.sFramebuffer);
}

bool GraphicsInstance::PrepareFrame(const int width, const int height)
{
	bool recreateSwapchain = renderer->PrepareFrame();

	return recreateSwapchain;
}

void GraphicsInstance::Render()
{
	//mGenUniformBuffer->UpdateUniform(&mGUbo, sizeof(mGUbo), 0, 1);
	//mGenUniformBuffer->UpdateUniform(&mLightUbo, sizeof(mLightUbo), 0, 2);

	//mGeometryUniformBuffer->UpdateUniform(&mGUbo, sizeof(mGUbo), 0, 1);
	UniformOptionsBuffer optionsBuffer;
	optionsBuffer.option = 5;

	DeferredUniformBuffer->UpdateUniform(&optionsBuffer, sizeof(UniformOptionsBuffer), 0, 4);
	DeferredUniformBuffer->UpdateUniform(&mLightUbo, sizeof(LightUbo) + (sizeof(SLight) * 8), 0, 5);


	renderer->Draw(mGBuffer.gCommandbuffer);
	renderer->Draw(mSBuffer.sCommandbuffer);
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

	// Light Transformation Matrix Generation
	float near_plane = 0.1f, far_plane = 100.0f;
	//glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	//lightProjection[1][1] *= -1;
	glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 96.0f);
	glm::mat4 lightView = glm::lookAt(glm::vec3(light.GetPosition()),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	mLightUbo.lightSpaceMatrix = lightProjection * lightView;


	return (LightIndex)(mLightUbo.countLights - 1);
}

void GraphicsInstance::UpdateLight(Light& light, LightIndex lightIndex)
{
	mLightUbo.light[lightIndex] = light.GetLightInformations();

	// Light Transformation Matrix Generation
	float near_plane = 0.1f, far_plane = 100.0f;
	//glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	//lightProjection[1][1] *= -1;
	glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
	glm::mat4 lightView = glm::lookAt(glm::vec3(light.GetPosition()),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	mLightUbo.lightSpaceMatrix = lightProjection * lightView;
}

void GraphicsInstance::BeginCommandBuffer(const int width, const int height)
{
	// main command Buffer
	commandBuffer->BeginCommandBuffer().

		SetViewport({ 0, 0, (float)width, (float)height, 0.0, 1.0 }).
		SetScissor({ 0, 0, (uint32_t)width, (uint32_t)height }).
		BeginRenderPass(renderPass, framebuffer, 0, 0, (uint32_t)width, (uint32_t)height).
		BindDescriptorSets(DeferredUniformBuffer, pipeline).
		BindPipeline(pipeline).
		Draw(3, 1, 0, 0).
		EndRenderPass().
		EndCommandBuffer();

	mSBuffer.sCommandbuffer->BeginCommandBuffer().
		BeginRenderPass(mSBuffer.sRenderPass, mSBuffer.sFramebuffer, 0, 0, FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE).
		SetViewport({ 0, 0, (float)FRAMEBUFFER_SIZE, (float)FRAMEBUFFER_SIZE, 0.0, 1.0 }).
		SetScissor({ 0, 0, (uint32_t)FRAMEBUFFER_SIZE, (uint32_t)FRAMEBUFFER_SIZE });

	mGBuffer.gCommandbuffer->BeginCommandBuffer().
		BeginRenderPass(mGBuffer.gRenderPass, mGBuffer.gFramebuffer, 0, 0, FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE).
		SetViewport({ 0, 0, (float)FRAMEBUFFER_SIZE, (float)FRAMEBUFFER_SIZE, 0.0, 1.0 }).
		SetScissor({ 0, 0, (uint32_t)FRAMEBUFFER_SIZE, (uint32_t)FRAMEBUFFER_SIZE });


	//ommandBuffer->BeginCommandBuffer().
	//	SetViewport({ 0, 0, (float)width, (float)height, 0.0, 1.0 }).
	//	SetScissor({ 0, 0, (uint32_t)width, (uint32_t)height }).
	//	BeginRenderPass(renderPass, framebuffer, 0, 0, width, height);
	
}

void GraphicsInstance::EndCommandBuffer()
{
	mSBuffer.sCommandbuffer->EndRenderPass();
	mSBuffer.sCommandbuffer->EndCommandBuffer();

	mGBuffer.gCommandbuffer->EndRenderPass();
	mGBuffer.gCommandbuffer->EndCommandBuffer();
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
	std::shared_ptr <Invision::ICommandBuffer> gBuffer = mParentAddr->GetGeometryCommandBuffer();
	std::shared_ptr <Invision::ICommandBuffer> shadowBuffer = mParentAddr->GetShadowCommandBuffer();

	// ShadowBuffer
	// Draw Mesh

	
	if (mesh.IsIndexed())
	{
		// SBuffer
		shadowBuffer->BindPipeline(mesh.GetShadowPipeline()).
			BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1).
			BindDescriptorSets(mesh.GetShadowUniformBufferObject(), mesh.GetShadowPipeline()).
			BindIndexBuffer(mesh.GetIndexBuffer(), Invision::INDEX_TYPE_UINT32).
			DrawIndexed(static_cast<uint32_t>(mesh.GetIndizes().size()), 1, 0, 0, 0);

		// GBuffer
		gBuffer->BindPipeline(mesh.GetPipeline()).
			BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1).
			BindDescriptorSets(mesh.GetGeneralUniformBufferObject(), mesh.GetPipeline()).
			BindIndexBuffer(mesh.GetIndexBuffer(), Invision::INDEX_TYPE_UINT32).
			//Draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0).
			DrawIndexed(static_cast<uint32_t>(mesh.GetIndizes().size()), 1, 0, 0, 0);

#ifdef INVISION_HL_DRAW_NORMALS

		gBuffer->BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1);
		gBuffer->BindIndexBuffer(mesh.GetIndexBuffer(), Invision::INDEX_TYPE_UINT32);
		gBuffer->BindPipeline(mesh.GetGeomPipeline());
		gBuffer->BindDescriptorSets(mesh.GetGeometryUniformBufferObject(), mesh.GetGeomPipeline());
		gBuffer->DrawIndexed(static_cast<uint32_t>(mesh.GetIndizes().size()), 1, 0, 0, 0);
#endif
	}
	else
	{
		shadowBuffer->BindPipeline(mesh.GetShadowPipeline()).
			BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1).
			BindDescriptorSets(mesh.GetShadowUniformBufferObject(), mesh.GetShadowPipeline()).
			Draw(static_cast<uint32_t>(mesh.GetVertizes().size()), 1, 0);

		gBuffer->BindPipeline(mesh.GetPipeline()).
			BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1).
			BindDescriptorSets(mesh.GetGeneralUniformBufferObject(), mesh.GetPipeline()).
			//Draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0).
			Draw(static_cast<uint32_t>(mesh.GetVertizes().size()), 1, 0);

#ifdef INVISION_HL_DRAW_NORMALS

		gBuffer->BindVertexBuffer({ mesh.GetVertexBuffer() }, 0, 1);
		gBuffer->BindPipeline(mesh.GetGeomPipeline());
		gBuffer->BindDescriptorSets(mesh.GetGeometryUniformBufferObject(), mesh.GetGeomPipeline());
		gBuffer->Draw(static_cast<uint32_t>(mesh.GetVertizes().size()), 1, 0);
#endif
	}

	
}