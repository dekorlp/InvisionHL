#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "Mesh.h"
#include "GraphicsInstance.h"

class InvisionHL
{
public:

	InvisionHL()
	{

	}

	void run(int width, int height, const char* title)
	{
		InitWindow(width, height, title);
		InitVulkan();

		//Init(drawingInstance); // called from user

		MainLoop();
		CleanUp();

		Destroy(); // called from user


	}

	virtual void PreInit(DrawingInstance& instance, int width, int height) = 0;
	virtual void Init(DrawingInstance& instance) = 0;
	virtual void Destroy() = 0;
	virtual void Render(DrawingInstance& instance, const int width, const int height) = 0;

public:
	void SetProjectionViewMatrix(glm::mat4 view, glm::mat4 proj, glm::vec3 cameraPosition);


private:
	// methods
	virtual void InitWindow(int width, int height, const char* title)
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		HWND hwnd = glfwGetWin32Window(window);
		gfxInstance.Init(hwnd, width, height);
		drawingInstance.SetParentAddr(gfxInstance);

		PreInit(drawingInstance, width, height);

		gfxInstance.BeginCommandBuffer(width, height);
		Init(drawingInstance); // called from User
		gfxInstance.EndCommandBuffer();
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<InvisionHL*>(glfwGetWindowUserPointer(window));
		app->resizeEventThrown = true;
	}

	virtual void InitVulkan()
	{

	}

	void RecreateSwapChain(int width, int height)
	{
		resizeEventThrown = false;
		std::cout << "Resize Event thrown" << std::endl;

		HWND hwnd = glfwGetWin32Window(window);
		gfxInstance.RecreateSwapchain(width, height);

		//buildCommandBuffer()
	}

	virtual void MainLoop()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();



			int width = 0, height = 0;
			glfwGetFramebufferSize(window, &width, &height);

			// render function
			if ((width < 0) || (height < 0))
				return;

			// my resize Code 
			if (resizeEventThrown == true)
			{
				RecreateSwapChain(width, height);

				// Build Command Buffer
				gfxInstance.BeginCommandBuffer(width, height);
				Init(drawingInstance); // called from user
				gfxInstance.EndCommandBuffer();
			}


			// render function
			gfxInstance.PrepareFrame(width, height);
			Render(drawingInstance, width, height); // called from user

			// Uniform Preparation


			// pass General UBO
			
			gfxInstance.Render();
			gfxInstance.SubmitFrame(width, height);

		}
	}

	virtual void CleanUp()
	{
		glfwDestroyWindow(window);

		glfwTerminate();
	}

	// vars
	GLFWwindow* window;
	GraphicsInstance gfxInstance; // Main Graphics Class
	DrawingInstance drawingInstance; // Derived Instance Graphics Class
	bool resizeEventThrown = false;
};