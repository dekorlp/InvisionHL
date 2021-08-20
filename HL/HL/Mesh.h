#pragma once

#include "Engine/InCommon.h"
#include "Engine/renderer/GraphicsFactory.h"

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GraphicsInstance;
class DrawingInstance;


struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 tex;
	glm::vec3 tangent;
};

typedef uint32_t Index;

struct Material {
	float diffuse;
	float specular;
	float shininess;
};

struct UniformBufferObject {
	__declspec(align(16)) Material mat;
	__declspec(align(16)) glm::mat4 model;
};


class Mesh
{
public:
	Mesh();
	Mesh(DrawingInstance& instance, std::vector<Vertex> vertices);
	Mesh(DrawingInstance& instance, std::vector<Vertex> vertices, std::vector<Index> indices, Invision::PolygonMode polygonMode = Invision::POLYGON_MODE_FILL, Invision::PrimitiveTopology primitiveTopology = Invision::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, float lineWidth = 1.0);

	std::vector<Vertex> GetVertizes();
	std::vector<Index> GetIndizes();

	std::shared_ptr <Invision::IVertexBuffer> GetVertexBuffer();
	std::shared_ptr<Invision::IIndexBuffer> GetIndexBuffer();
	std::shared_ptr<Invision::IPipeline> GetPipeline();
	std::shared_ptr<Invision::IPipeline> GetGeomPipeline();
	bool IsIndexed();

	void UpdateUniform(DrawingInstance& instance, glm::mat4 modelMatrix);
	void SetModelMatrix(glm::mat4 model);
	glm::mat4 GetModelMatrix();

	static Mesh CreateCube(DrawingInstance& instance, float width, float height, float depth);
	static Mesh CreatePyramid(DrawingInstance& instance, float width, float height, float depth);
	static Mesh CreateCylinder(DrawingInstance& instance, float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount);
	static Mesh CreateGeoSphere(DrawingInstance& instance, float radius, uint32_t subDivisions);
	static Mesh CreateSphere(DrawingInstance& instance, float radius, unsigned int stacks, unsigned int slices);
	static Mesh CreatePlane(DrawingInstance& instance, float width, float depth);
	static Mesh CreateGrid(DrawingInstance& instance, float width, float depth, unsigned int m, unsigned int n, float lineWidth = 1.0f);

	void AddChildMesh(Mesh& mesh)
	{
		mChildMeshes.push_back(&mesh);
		mesh.SetParent(*this);
	}

	std::shared_ptr <Invision::IUniformBuffer> GetGeneralUniformBufferObject()
	{
		return mGenUniformBuffer;
	}

	std::shared_ptr <Invision::IUniformBuffer> GetGeometryUniformBufferObject()
	{
		return mGeometryUniformBuffer;
	}

	void SetParent(Mesh& mesh)
	{
		this->parent = &mesh;
	}



private:
	std::vector<Vertex> mVertices;
	std::vector<Index> mIndizes;
	bool isIndexed;
	glm::mat4 mModelMat;

	std::vector<Mesh*> mChildMeshes;
	Mesh* parent = nullptr;

	std::shared_ptr <Invision::IUniformBuffer> mGenUniformBuffer;
	std::shared_ptr <Invision::IUniformBuffer> mGeometryUniformBuffer;

	// becomes user 
	std::shared_ptr <Invision::IVertexBuffer> vertexBuffer;
	std::shared_ptr <Invision::IIndexBuffer> indexBuffer;
	std::shared_ptr <Invision::IPipeline> pipeline;

	std::shared_ptr <Invision::IPipeline> geomPipeline;

	std::vector<char> readFile(const std::string& filename);

	static void Subdivide(std::vector<Vertex> &vertizes, std::vector<uint32_t> &indizes);
	static Vertex Mesh::MidPoint(const Vertex& v0, const Vertex& v1);
};