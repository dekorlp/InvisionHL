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
/*class App : public InvisionHL
{

	void PreInit(DrawingInstance& instance, int width, int height)
	{
		const std::vector<Vertex> vertices = {
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

		std::cout << "Init called" << std::endl;
		
		//quad = Mesh::CreateGrid(instance, 10.f, 10.f, 10, 10, 2.0);
		//lineWidth2 = Mesh::CreateGrid(instance, 10.f, 10.f, 100, 100, 1.0);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreatePlane(instance, 2.f, 2.f);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreateSphere(instance, 0.5f, 50, 50);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreateGeoSphere(instance, 0.5f, 2.5f);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreateCylinder(instance, 0.5f, 0.3f, 2.0f, 20, 20);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreatePyramid(instance, 1, 2, 1);// Mesh(instance, vertices, indices);

		quad = Mesh::CreateCube(instance, 1, 1, 1);
		quad2 = Mesh::CreatePyramid(instance, 2, 1, 1);

		quad.AddChildMesh(quad2);

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
		instance.BindMesh(quad2);		
	}

	void Destroy() override
	{
		std::cout << "Destroy called" << std::endl;
	}

	void Render(DrawingInstance& instance, const int width, const int height) override
	{	
		std::cout << "Render " << std::endl;

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		//quad.SetModelMatrix(glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		//quad.UpdateUniform(instance, quad.GetModelMatrix());

		//lineWidth2.SetModelMatrix(glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		//lineWidth2.UpdateUniform(instance, lineWidth2.GetModelMatrix());

		
		// chield
		quad2.SetModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 0.0f, 0.0f)));
		quad2.UpdateUniform(instance, quad2.GetModelMatrix());

		// parent
		quad.SetModelMatrix(glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
		quad.UpdateUniform(instance, quad.GetModelMatrix());

	}


private:
	Mesh quad;
	Mesh quad2;
	
	Light light;

	glm::mat4 view;
	glm::mat4 proj;

	
};*/

// Pinguin Example
class App : public InvisionHL
{

	void PreInit(DrawingInstance& instance, int width, int height)
	{
		std::cout << "Init called" << std::endl;

		const std::vector<Vertex> vertices = {
			{ { -1.5f, -0.7f, 0.0f },{ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { 1.5f, -0.7f, 0.0f },{ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { 1.5f, 0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
			//{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },

		};

		const std::vector<uint32_t> indices = {
			0, 1, 2//, 2, 3, 0
			//4, 5, 6, 6, 7, 4
		};



		//quad = Mesh::CreateGrid(instance, 10.f, 10.f, 10, 10, 2.0);
		//lineWidth2 = Mesh::CreateGrid(instance, 10.f, 10.f, 100, 100, 1.0);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreatePlane(instance, 2.f, 2.f);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreateSphere(instance, 0.5f, 50, 50);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreateGeoSphere(instance, 0.5f, 2.5f);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreateCylinder(instance, 0.5f, 0.3f, 2.0f, 20, 20);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreatePyramid(instance, 1, 2, 1);// Mesh(instance, vertices, indices);

		//quad = Mesh::CreateCube(instance, 1, 1, 1);
		//quad2 = Mesh::CreatePyramid(instance, 2, 1, 1);

		//quad.AddChildMesh(quad2);

		plane = Mesh::CreatePlane(instance, 200.0f, 200.0f, glm::vec3(1.0f, 1.0f, 0.0f));
		leftFeet = Mesh::CreatePyramid(instance, 1.0, 0.5, 4.0, glm::vec3(1.0f, 0.0f, 0.0f));
		rightFeet = Mesh::CreatePyramid(instance, 1.0, 0.5, 4.0, glm::vec3(1.0f, 0.0f, 0.0f));
		bottomCylinder = Mesh::CreateCylinder(instance, 1.5, 2.0, 1.0, 100, 100, glm::vec3(1.0f, 0.0f, 0.0f));
		bodyCylinder = Mesh::CreateCylinder(instance, 2.0, 2.0, 8.0, 100, 100, glm::vec3(1.0f, 0.0f, 0.0f));
		topCylinder = Mesh::CreateCylinder(instance, 2.0, 1.5, 1.0, 100, 100, glm::vec3(1.0f, 0.0f, 0.0f));
		head = Mesh::CreateSphere(instance, 2.0f, 50, 50, glm::vec3(1.0f, 1.0f, 1.0f));
		nose = Mesh::CreateCylinder(instance, 0.5, 0.0, 4.0, 100, 100, glm::vec3(1.0f, 1.0f, 1.0f));
		leftHand = Mesh(instance, vertices, indices);
		rightHand = Mesh(instance, vertices, indices);
		

		leftFeet.AddChildMesh(rightFeet);
		leftFeet.AddChildMesh(leftHand);
		leftFeet.AddChildMesh(rightHand);
		leftFeet.AddChildMesh(head);
		leftFeet.AddChildMesh(bottomCylinder);
		leftFeet.AddChildMesh(bodyCylinder);
		leftFeet.AddChildMesh(topCylinder);
		leftFeet.AddChildMesh(nose);

		view = glm::lookAt(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 100.0f);
		proj[1][1] *= -1;
		SetProjectionViewMatrix(view, proj, glm::vec3(2.0f, 2.0f, 2.0f));

		light = Light(glm::vec3(3.0f, 6.0f, 15.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		LightIndex lgtIndex = instance.AddLight(light);
		light.SetStrength(1);
		instance.UpdateLight(light, lgtIndex);

		//light1 = Light(glm::vec3(0.0f, -20.0f, 10.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		//LightIndex lgtIndex1 = instance.AddLight(light1);
		//light1.SetStrength(8);
		//instance.UpdateLight(light1, lgtIndex1);
	}

	void Init(DrawingInstance& instance) override
	{
		instance.BindMesh(plane);
		instance.BindMesh(leftFeet);
		instance.BindMesh(rightFeet);
		instance.BindMesh(bottomCylinder);
		instance.BindMesh(bodyCylinder);
		instance.BindMesh(topCylinder);
		instance.BindMesh(head);
		instance.BindMesh(nose);
		instance.BindMesh(leftHand);
		instance.BindMesh(rightHand);

	}

	void Destroy() override
	{
		std::cout << "Destroy called" << std::endl;
	}

	void Render(DrawingInstance& instance, const int width, const int height) override
	{
		std::cout << "Render " << std::endl;

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		// objects

		plane.SetModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
		plane.UpdateUniform(instance, plane.GetModelMatrix());

		leftFeet.SetModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.1f)));
		leftFeet.UpdateUniform(instance, leftFeet.GetModelMatrix());

		//leftFeet.SetModelMatrix(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.1f)), glm::radians(-200.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		//leftFeet.UpdateUniform(instance, leftFeet.GetModelMatrix());

		rightFeet.SetModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)));
		rightFeet.UpdateUniform(instance, rightFeet.GetModelMatrix());

		bottomCylinder.SetModelMatrix(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 1.5f)), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		bottomCylinder.UpdateUniform(instance, bottomCylinder.GetModelMatrix());

		bodyCylinder.SetModelMatrix(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 6.0f)), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		bodyCylinder.UpdateUniform(instance, bodyCylinder.GetModelMatrix());

		topCylinder.SetModelMatrix(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 10.5f)), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		topCylinder.UpdateUniform(instance, topCylinder.GetModelMatrix());

		head.SetModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 12.4f)));
		head.UpdateUniform(instance, head.GetModelMatrix());

		nose.SetModelMatrix(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 12.0f)), glm::radians(-180.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		nose.UpdateUniform(instance, nose.GetModelMatrix());

		leftHand.SetModelMatrix(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0f, 8.0f)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(-75.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		leftHand.UpdateUniform(instance, leftHand.GetModelMatrix());

		rightHand.SetModelMatrix(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 8.0f)), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(-75.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		rightHand.UpdateUniform(instance, rightHand.GetModelMatrix());

		
	}


private:
	Mesh plane;
	Mesh leftFeet;
	Mesh rightFeet;
	Mesh bottomCylinder;
	Mesh bodyCylinder;
	Mesh topCylinder;
	Mesh head;
	Mesh nose;
	Mesh leftHand;
	Mesh rightHand;
	


	Light light;
	Light light1;

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
