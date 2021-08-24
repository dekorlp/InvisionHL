#pragma once
#include "Engine/InCommon.h"
#include "Engine/renderer/GraphicsFactory.h"
#include "Engine/common/StopWatch.h"
#include "Engine/input/IKeyboard.h"
#include "Engine/input/WindowsKeyboard.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Light.h"

class Mesh;
class Light;

struct SLight;

struct GBuffer
{
	//std::shared_ptr <Invision::IPipeline> gPipeline;
	std::shared_ptr <Invision::IRenderPass> gRenderPass;
	std::shared_ptr <Invision::IFramebuffer> gFramebuffer;
	std::shared_ptr <Invision::ICommandBuffer> gCommandbuffer;
	std::shared_ptr <Invision::ITexture> positionsAttachment;
	std::shared_ptr <Invision::ITexture> albedoAttachment;
	std::shared_ptr <Invision::ITexture> normalAttachment;
	std::shared_ptr <Invision::ITexture> depthAttachment;
};

struct ShadowBuffer
{
	//std::shared_ptr <Invision::IPipeline> sPipeline;
	std::shared_ptr <Invision::IRenderPass> sRenderPass;
	std::shared_ptr <Invision::IFramebuffer> sFramebuffer;
	std::shared_ptr <Invision::ICommandBuffer> sCommandbuffer;
	std::shared_ptr <Invision::ITexture> sDepthAttachment;
};

#define FRAMEBUFFER_SIZE 2048

/*struct SLight
{
	__declspec(align(16)) glm::vec4 position;
	__declspec(align(16)) glm::vec4 color;
	__declspec(align(16)) float strength;
	//__declspec(align(16)) float direction;
	//__declspec(align(16)) float falloffStart;
	//__declspec(align(16)) float falloffEnd;
	//__declspec(align(16)) float spotPower;
};*/

struct LightUbo {
	__declspec(align(16)) SLight light[8];
	__declspec(align(16)) int countLights;
	__declspec(align(16)) glm::mat4 lightSpaceMatrix;
};

struct GeneralUbo {
	
	__declspec(align(16)) glm::mat4 view;
	__declspec(align(16)) glm::mat4 proj;
	__declspec(align(16)) glm::vec3 viewPos;
	
};

#define INVISION_HL_DRAW_NORMALS

class GraphicsInstance
{
	public:
		GraphicsInstance();
		void Init(HWND hwnd, int width, int height);
		void RecreateSwapchain(const int width,const int height);

		bool PrepareFrame(const int width, const int height);
		void Render();
		bool SubmitFrame(const int width, const int height);
		void SetProjectionViewMatrix(glm::mat4 view, glm::mat4 proj, glm::vec3 cameraPosition);
		LightIndex AddLight(Light& light);
		void UpdateLight(Light& light, LightIndex lightIndex);

		void BeginCommandBuffer(const int width, const int height);
		void EndCommandBuffer();

		void Destroy();
		~GraphicsInstance();

		std::shared_ptr <Invision::IGraphicsInstance> GetGraphicsInstance()
		{
			return graphicsInstance;
		}

		std::shared_ptr <Invision::IRenderPass> GetRenderPass()
		{
			return mGBuffer.gRenderPass;
		}

		std::shared_ptr <Invision::IRenderPass> GetShadowRenderPass()
		{
			return mSBuffer.sRenderPass;
		}

		std::shared_ptr <Invision::ICommandBuffer> GetGeometryCommandBuffer()
		{
			return mGBuffer.gCommandbuffer;
		}

		std::shared_ptr <Invision::ICommandBuffer> GetShadowCommandBuffer()
		{
			return mSBuffer.sCommandbuffer;
		}



		GeneralUbo GetGeneralUbo()
		{
			return mGUbo;
		}

		LightUbo GetLightUbo()
		{
			return mLightUbo;
		}

	private:
		Invision::Log log;
		HWND mHwnd;

		std::shared_ptr <Invision::IGraphicsEngine> graphicsEngine;
		std::shared_ptr <Invision::IGraphicsInstance> graphicsInstance;
		std::shared_ptr <Invision::IRenderer> renderer;

		std::shared_ptr <Invision::IRenderPass> renderPass;
		std::shared_ptr <Invision::IFramebuffer> framebuffer;
		std::shared_ptr <Invision::ICommandBuffer> commandBuffer;

		std::shared_ptr <Invision::IPipeline> pipeline;
		std::shared_ptr <Invision::IUniformBuffer> DeferredUniformBuffer;

		struct UniformOptionsBuffer {
			int option;
		};


		GBuffer mGBuffer;
		ShadowBuffer mSBuffer;

		GeneralUbo mGUbo;
		LightUbo mLightUbo;

		std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}
};

class DrawingInstance : public GraphicsInstance
{
public:
	DrawingInstance()
	{

	}

	DrawingInstance(GraphicsInstance& parentAddr) 
	{
		int test = 0;
		mParentAddr = &parentAddr;
	}
	std::shared_ptr <Invision::IGraphicsInstance> GetGraphicsInstance()
	{
		return mParentAddr->GetGraphicsInstance();
	}

	std::shared_ptr <Invision::IRenderPass> GetRenderPass()
	{
		return mParentAddr->GetRenderPass();
	}

	std::shared_ptr <Invision::IRenderPass> GetShadowRenderPass()
	{
		return mParentAddr->GetShadowRenderPass();
	}

	void SetParentAddr(GraphicsInstance& parentAddr)
	{
		mParentAddr = &parentAddr;
	}

	LightIndex AddLight(Light& light)
	{
		return mParentAddr->AddLight(light);
	}

	void UpdateLight(Light& light, LightIndex lightIndex)
	{
		mParentAddr->UpdateLight(light, lightIndex);
	}

	GeneralUbo GetGeneralUbo()
	{
		return mParentAddr->GetGeneralUbo();
	}

	LightUbo GetLightUbo()
	{
		return mParentAddr->GetLightUbo();
	}

	void BindMesh(Mesh &mesh);

private:
	GraphicsInstance* mParentAddr;
};