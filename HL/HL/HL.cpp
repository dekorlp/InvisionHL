// HL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "Mesh.h"
#include "Light.h"
#include "GraphicsInstance.h"

#include "InvisionHL.h"

// default App Example
class App : public InvisionHL
{

	void PreInit(DrawingInstance& instance, int width, int height)
	{
		// With Indices
		const std::vector<Vertex> verticesWithIndices = {
			{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
			{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
			{ { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f } },

			{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
			{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
			{ { 0.5f, 0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, 0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f } }
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

		// Without Indices
		const std::vector<Vertex> verticesWithoutIndices = {
				{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
				{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
				{ { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
				{ { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
				{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f } },
				{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f } },

				{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
				{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
				{ { 0.5f, 0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f } },
				{ { 0.5f, 0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f } },
				{ { -0.5f, 0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f } },
				{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } }
		};


		std::cout << "Init called" << std::endl;
		
		// Without indices
		//quad = Mesh(instance, verticesWithoutIndices);

		// With indices 
		quad = Mesh(instance, verticesWithIndices, indices);


		view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		proj[1][1] *= -1;
		SetProjectionViewMatrix(view, proj, glm::vec3(2.0f, 2.0f, 2.0f));

		light = Light(glm::vec3(4.0f, 4.0f, 7.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		LightIndex lgtIndex = instance.AddLight(light);
		light.SetStrength(8);
		instance.UpdateLight(light, lgtIndex);
	}

	void Init(DrawingInstance& instance) override
	{
		instance.BindMesh(quad);	
	}

	void Destroy() override
	{
		std::cout << "Destroy called" << std::endl;
	}

	void Render(DrawingInstance& instance, const int width, const int height) override
	{	
		instance.SetLightingOption(LIGHTING_OPTION_DIFFUSE);

		std::cout << "Render " << std::endl;

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		// Model Translations
		quad.SetModelMatrix(glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f)));
		quad.UpdateUniform(instance, quad.GetModelMatrix());

	}


private:
	Mesh quad;
	
	Light light;

	glm::mat4 view;
	glm::mat4 proj;

	
};

int main()
{
	App app;

	try {
		app.run(1024, 768, "Vulkan Window");
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}