#include "GraphicsInstance.h"

#include "Mesh.h"


Mesh::Mesh()
{

}

Mesh::Mesh(DrawingInstance& instance, std::vector<Vertex> vertices, float shininess)
{
	mVertices = vertices;
	isIndexed = false;
	mShininess = shininess;
}

Mesh::Mesh(DrawingInstance& instance, std::vector<Vertex> vertices, std::vector<Index> indices, float shininess, Invision::PolygonMode polygonMode, Invision::PrimitiveTopology primitiveTopology, float lineWidth)
{
	mVertices = vertices;
	mIndizes = indices;
	mShininess = shininess;

	std::shared_ptr <Invision::IGraphicsInstance> graphicsInstance = instance.GetGraphicsInstance();
	// user code
	vertexBuffer = graphicsInstance->CreateVertexBuffer();
	indexBuffer = graphicsInstance->CreateIndexBuffer();
	pipeline = graphicsInstance->CreatePipeline(&Invision::PipelineProperties(primitiveTopology, polygonMode, Invision::CULL_MODE_NONE, Invision::FRONT_FACE_COUNTER_CLOCKWISE, lineWidth));
	std::shared_ptr<Invision::IVertexBindingDescription> verBindingDescr = graphicsInstance->CreateVertexBindingDescription();
	verBindingDescr->CreateVertexBinding(0, sizeof(Vertex), Invision::VERTEX_INPUT_RATE_VERTEX)
		->CreateAttribute(0, Invision::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position))
		.CreateAttribute(1, Invision::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
		.CreateAttribute(2, Invision::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));

	vertexBuffer->CreateBuffer(vertices.data(), sizeof(vertices[0]) * vertices.size(), 0, verBindingDescr);

	indexBuffer->CreateIndexBuffer(sizeof(indices[0]) * indices.size(), indices.data());

	// Create Uniform Buffer
	mGenUniformBuffer = graphicsInstance->CreateUniformBuffer();
	mGenUniformBuffer->CreateUniformBinding(0, 0, 1, Invision::SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject))
		.CreateUniformBinding(0, 1, 1, Invision::SHADER_STAGE_VERTEX_BIT | Invision::SHADER_STAGE_FRAGMENT_BIT, sizeof(GeneralUbo)).CreateUniformBuffer();

	mGeometryUniformBuffer = graphicsInstance->CreateUniformBuffer();
	mGeometryUniformBuffer->CreateUniformBinding(0, 0, 1, Invision::SHADER_STAGE_VERTEX_BIT | Invision::SHADER_STAGE_GEOMETRY_BIT, sizeof(UniformBufferObject))
		.CreateUniformBinding(0, 1, 1, Invision::SHADER_STAGE_VERTEX_BIT | Invision::SHADER_STAGE_GEOMETRY_BIT | Invision::SHADER_STAGE_FRAGMENT_BIT, sizeof(GeneralUbo)).CreateUniformBuffer();

	auto vertShaderCode = readFile("C:/Repository/InvisionHL/HL/HL/Shader/gBuffer.vert.spv");
	auto fragShaderCode = readFile("C:/Repository/InvisionHL/HL/HL/Shader/gBuffer.frag.spv");
	pipeline->AddUniformBuffer(mGenUniformBuffer);
	pipeline->AddShader(vertShaderCode, Invision::SHADER_STAGE_VERTEX_BIT);
	pipeline->AddShader(fragShaderCode, Invision::SHADER_STAGE_FRAGMENT_BIT);
	pipeline->AddVertexDescription(verBindingDescr);
	pipeline->CreatePipeline(instance.GetGeometryRenderPass());
	isIndexed = true;

	// Deferred Shadow Shading
	mShadowUniformBuffer = graphicsInstance->CreateUniformBuffer();
	mShadowUniformBuffer->CreateUniformBinding(0, 0, 1, Invision::SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject))
		.CreateUniformBinding(0, 1, 1, Invision::SHADER_STAGE_VERTEX_BIT, sizeof(LightUbo) + (sizeof(SLight) * 8))
		.CreateUniformBuffer();
	mShadowPipeline = graphicsInstance->CreatePipeline();
	mShadowPipeline->AddUniformBuffer(mShadowUniformBuffer);
	auto vertShaderCode2 = readFile(std::string("C:/Repository/InvisionHL/HL/HL/Shader/shadow.vert.spv"));
	mShadowPipeline->AddShader(vertShaderCode2, Invision::SHADER_STAGE_VERTEX_BIT);
	mShadowPipeline->AddVertexDescription(verBindingDescr);
	mShadowPipeline->CreatePipeline(instance.GetShadowRenderPass());

	// Create geometry Shader Pipeline
	auto vertShaderNormalCode = readFile("C:/Repository/InvisionHL/HL/HL/Shader/DebugGeom/normal.vert.spv");
	auto geomShaderNormalCode = readFile("C:/Repository/InvisionHL/HL/HL/Shader/DebugGeom/normal.geom.spv");
	auto fragShaderNormalCode = readFile("C:/Repository/InvisionHL/HL/HL/Shader/DebugGeom/normal.frag.spv");
	geomPipeline = graphicsInstance->CreatePipeline(&Invision::PipelineProperties(Invision::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, Invision::POLYGON_MODE_FILL, Invision::CULL_MODE_FRONT_BIT, Invision::FRONT_FACE_COUNTER_CLOCKWISE, 1.0f));
	geomPipeline->AddUniformBuffer(mGeometryUniformBuffer);
	geomPipeline->AddShader(vertShaderNormalCode, Invision::SHADER_STAGE_VERTEX_BIT);
	geomPipeline->AddShader(geomShaderNormalCode, Invision::SHADER_STAGE_GEOMETRY_BIT);
	geomPipeline->AddShader(fragShaderNormalCode, Invision::SHADER_STAGE_FRAGMENT_BIT);
	geomPipeline->AddVertexDescription(verBindingDescr);
	geomPipeline->CreatePipeline(instance.GetDeferredRenderPass());


	

}

void Mesh::UpdateUniform(DrawingInstance& instance, glm::mat4 modelMatrix)
{
	// update current mesh
	UniformBufferObject ubo = {};

	ubo.mat.shininess = mShininess;

	if (parent)
	{
		ubo.model = parent->GetModelMatrix() * modelMatrix;
	}
	else
	{
		ubo.model = modelMatrix;
	}

	// update child meshes
	for (unsigned int i = 0; i < mChildMeshes.size(); i++)
	{
		mChildMeshes[i]->SetModelMatrix(ubo.model);
		mChildMeshes[i]->UpdateUniform(instance, mChildMeshes[i]->GetModelMatrix());
	}
	
	
	// Shadow Uniform Buffer
	mShadowUniformBuffer->UpdateUniform(&ubo, sizeof(ubo), 0, 0);
	mShadowUniformBuffer->UpdateUniform(&instance.GetLightUbo(), sizeof(instance.GetLightUbo()) + (sizeof(SLight) * 8), 0, 1);

	mGenUniformBuffer->UpdateUniform(&instance.GetGeneralUbo(), sizeof(instance.GetGeneralUbo()), 0, 1);
	//mGenUniformBuffer->UpdateUniform(&instance.GetLightUbo(), sizeof(instance.GetLightUbo()), 0, 2);
	
	mGeometryUniformBuffer->UpdateUniform(&instance.GetGeneralUbo(), sizeof(instance.GetGeneralUbo()), 0, 1);
	mGenUniformBuffer->UpdateUniform(&ubo, sizeof(ubo), 0, 0);
	mGeometryUniformBuffer->UpdateUniform(&ubo, sizeof(ubo), 0, 0);
}

void Mesh::SetModelMatrix(glm::mat4 model)
{
	mModelMat = model;
}

glm::mat4 Mesh::GetModelMatrix()
{
	return mModelMat;
}

std::vector<char> Mesh::readFile(const std::string& filename)
{
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

std::vector<Vertex> Mesh::GetVertizes()
{
	return mVertices;
}

std::vector<Index> Mesh::GetIndizes()
{
	return mIndizes;
}

std::shared_ptr <Invision::IVertexBuffer> Mesh::GetVertexBuffer()
{
	return vertexBuffer;
}

std::shared_ptr<Invision::IIndexBuffer> Mesh::GetIndexBuffer()
{
	return indexBuffer;
} 

Mesh Mesh::CreateCube(DrawingInstance& instance, float width, float height, float depth, glm::vec3 color, float shininess)
{
	// https://github.com/jjuiddong/Introduction-to-3D-Game-Programming-With-DirectX11/blob/master/Common/GeometryGenerator.cpp#L26

	width = width * 0.5f;
	height = height * 0.5f;
	depth = depth * 0.5f;

	const std::vector<uint32_t> indices = {
		// BOTTOM
		0, 1, 2,
		2, 3, 1,
		
		// TOP
		4, 5, 6,
		6, 7, 5,

		// FRONT
		8, 9, 10,
		10, 11, 9,

		// BACK
		12, 13, 14,
		14, 15, 13,

		// LSIDE
		16, 17, 18,
		18, 19, 17,

		// RSIDE
		20, 21, 22,
		22, 23, 21
	};

	const std::vector<Vertex> vertices = {
		// BOTTOM
		{ { -width, +height, -depth }, color, { 0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} , {-1.0f, 0.0f, 0.0f} }, //0
		{ { width,  height, -depth }, color, { 0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}, //1
		{ { -width, -height, -depth }, color, { 0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} }, //2
		{ { width, -height, -depth }, color, { 0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} }, //3

		//TOP
		{ { -width, +height, +depth },color, { 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }, //4 
		{ { width,  height, +depth },color, { 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }, //5
		{ { -width, -height, +depth },color, { 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} }, //6
		{ { width, -height, +depth },color, { 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} }, //7

		// FRONT
		{ { -width,  +height, +depth, },color, { 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} }, //8
		{ { width,  +height, depth },color, { 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} }, //9
		{ { -width,  +height , -depth },color, { 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} }, //10
		{ { width,  +height , -depth },color, { 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} }, //11

		// BACK
		{ { -width,  -height, +depth, },color, { 0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }, //12
		{ { width,  -height, depth },color, { 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} }, //13
		{ { -width,  -height , -depth },color, { 0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} }, //14
		{ { width,  -height , -depth },color, { 0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} }, //15

		// LSIDE
		{ { width,  height, +depth, },color, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} }, //16
		{ { width,  -height, depth },color, { 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} }, //17
		{ { width,  height , -depth },color, { 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} }, //18
		{ { width,  -height , -depth },color, { 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} }, //19

		// RSIDE
		{ { -width,  height, +depth, },color, { -1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} }, //20
		{ { -width,  -height, depth },color, { -1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f} }, //21
		{ { -width,  height , -depth },color, { -1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} }, //22
		{ { -width,  -height , -depth },color, { -1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} }, //23
	};

	

	return Mesh(instance, vertices, indices, shininess);
}

Mesh Mesh::CreatePyramid(DrawingInstance& instance, float width, float height, float depth, glm::vec3 color, float shininess)
{
	width = width * 0.5f;
	height = height * 0.5f;
	depth = depth * 0.5f;

	const std::vector<Vertex> vertices = {
		// BACK
		{ { -width, +depth, -height },color, { 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0, 0.0f, 0.0f} },
		{ { +width, +depth, -height },color, { 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {-1.0, 0.0f, 0.0f} },
		{ { +0,     +0,     +height },color, { 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {-1.0, 0.0f, 0.0f} },
		// FRONT
		{ { -width, -depth, -height },color, { 0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0, 0.0f, 0.0f} },
		{ { width, -depth, -height },color, { 0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0, 0.0f, 0.0f} },
		{ { +0,    +0,     +height },color, { 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0, 0.0f, 0.0f} },

		// LEFT
		{ { -width, +depth, -height },color, { 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0, -1.0f, 0.0f} },
		{ { -width, -depth, -height },color, { 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0, -1.0f, 0.0f} },
		{ { +0,     +0,     +height },color, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0, -1.0f, 0.0f} },

		// RIGHT
		{ { +width, +depth, -height },color, { -1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0, 1.0f, 0.0f} },
		{ { +width, -depth, -height },color, { -1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0, 1.0f, 0.0f} },
		{ { +0,     +0,     +height },color, { -1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0, 1.0f, 0.0f} },

		// BOTTOM
		{ { -width, +depth, -height },color, { 0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {-1.0, 0.0f, 0.0f} },
		{ { +width, +depth, -height },color, { 0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {-1.0, 0.0f, 0.0f} },
		{ { +width, -depth, -height },color, { 0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {-1.0, 0.0f, 0.0f} },
		{ { -width, -depth, -height },color, { 0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {-1.0, 0.0f, 0.0f} },
	};

	const std::vector<uint32_t> indices = {
		// BACK
		0, 1, 2,

		// FRONT
		3, 4, 5,

		// LEFT
		6, 7, 8,

		// RIGHT
		9, 10, 11,

		// BOTTOM
		12, 13, 14,
		14, 12, 15

	};

	return Mesh(instance, vertices, indices, shininess);
}

Mesh Mesh::CreateCylinder(DrawingInstance& instance, float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, glm::vec3 color, float shininess)
{
	// 3D Game Programming with DirectX 12 - P. 275ff 
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	float stackHeight = height / stackCount;
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	uint32_t ringCount = stackCount + 1;

	// Vertizes
	for (uint32_t i = 0; i < ringCount; ++i)
	{
		float y = -0.5f*height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		float dTheta = 2.0f*glm::pi<float>()/sliceCount;

		for (uint32_t j = 0; j <= sliceCount; ++j)
		{
			Vertex vertex;

			float c = cosf(j*dTheta);
			float s = sinf(j*dTheta);

			vertex.position = glm::vec3(r*c, -y, r*s);
			vertex.tex.x = (float)j / sliceCount;
			vertex.tex.y = 1.0f - (float)i / stackCount;

			vertex.tangent = glm::vec3(-s, 0.0f, -c);

			float dr = bottomRadius - topRadius;
			glm::vec3 bitangent(dr*c, -height, -(dr*s));

			vertex.color = color;
			vertex.normal = glm::normalize(glm::cross(vertex.tangent, bitangent));

			vertices.push_back(vertex);
		}
	}

	uint32_t ringVertexCount = sliceCount + 1;

	// Indizes
	for (uint32_t i = 0; i < stackCount; ++i)
	{
		for (uint32_t j = 0; j < sliceCount; ++j)
		{
			indices.push_back(i*ringVertexCount + j);
			indices.push_back((i+1) * ringVertexCount + j);
			indices.push_back((i+1) * ringVertexCount + j+1);

			indices.push_back(i * ringVertexCount + j);
			indices.push_back((i+1) * ringVertexCount + j+1);
			indices.push_back(i*ringVertexCount + j+1);
		}
	}

	// Build Top Cap
	uint32_t baseIndex = (uint32_t)vertices.size();

	float y = 0.5f*height;
	float dTheta = 2.0f*glm::pi<float>() / sliceCount;

	for(uint32_t i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i*dTheta);
		float z = topRadius * sinf(i*dTheta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		Vertex vert;
		vert.position = glm::vec3(x, -y, z);
		vert.tex = glm::vec2(u, v);
		vert.normal = glm::vec3(0.0, -1.0, 0.0);
		vert.tangent = glm::vec3(1.0, 0.0, 0.0);
		vert.color = color;

		vertices.push_back(vert);
	}
	Vertex TopCenter;
	TopCenter.position = glm::vec3(0.0f, -y, 0.0f);
	TopCenter.color = color;
	TopCenter.normal = glm::vec3(0.0, -1.0, 0.0);
	TopCenter.tangent = glm::vec3(1.0, 0.0, 0.0);
	TopCenter.tex = glm::vec2(0.5f, 0.5f);
	vertices.push_back(TopCenter);

	uint32_t topCenterIndex = (uint32_t)vertices.size()-1;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		indices.push_back(topCenterIndex);
		indices.push_back(baseIndex + i+1);
		indices.push_back(baseIndex + i);
	}

	//Build Bottom Cap
	baseIndex = (uint32_t)vertices.size();
	for (uint32_t i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i*dTheta);
		float z = bottomRadius * sinf(i*dTheta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		Vertex vert;
		vert.position = glm::vec3(x, y, z);
		vert.color = color;
		vert.normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vert.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
		vert.tex = glm::vec2(u, v);

		vertices.push_back(vert);
	}

	Vertex BottomCenter;
	BottomCenter.position = glm::vec3(0.0f, y, 0.0f);
	BottomCenter.color = color;
	BottomCenter.normal = glm::vec3(0.0f, 1.0f, 0.0f);
	BottomCenter.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
	BottomCenter.tex = glm::vec2(0.5f, 0.5f);
	vertices.push_back(BottomCenter);

	uint32_t bottomCenterIndex = (uint32_t)vertices.size() - 1;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		indices.push_back(bottomCenterIndex);
		indices.push_back(baseIndex + i + 1);
		indices.push_back(baseIndex + i);
	}

	return Mesh(instance, vertices, indices, shininess);
}

Mesh Mesh::CreateGeoSphere(DrawingInstance& instance, float radius, uint32_t subDivisions, glm::vec3 color, float shininess)
{
	// 3D Game Programming with DirectX 12 - P. 280f
	// https://schneide.blog/2016/07/15/generating-an-icosphere-in-c/
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// Put a cap for subdivisions
	subDivisions = std::min<uint32_t>(subDivisions, 6u);

	// Approximation
	float X = 0.525731f;
	float Z = 0.850651f;

	glm::vec3 pos[12] =
	{
		glm::vec3(-X, 0.0f, Z), glm::vec3(X, 0.0f, Z),
		glm::vec3(-X, 0.0f, -Z), glm::vec3(X, 0.0f, -Z),
		glm::vec3(0.0f, Z, X), glm::vec3(0.0f, Z, -X),
		glm::vec3(0.0f, -Z, X), glm::vec3(0.0f, -Z, -X),
		glm::vec3(Z, X, 0.0f), glm::vec3(-Z, X, 0.0f),
		glm::vec3(Z, -X, 0.0f), glm::vec3(-Z, -X, 0.0f)
	};

	uint32 k[60] =
	{
		1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4,
		1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9, 11,2,7
	};

	vertices.resize(12);
	indices.assign(&k[0], &k[60]);

	for (uint32_t i = 0; i < 12; ++i)
	{
		vertices[i].position = pos[i];
		vertices[i].color = color;
	}

	for (uint32_t i = 0; i < subDivisions; ++i)
		Subdivide(vertices, indices);

	for (uint32_t i = 0; i < vertices.size(); ++i)
	{
		glm::vec3 n = glm::normalize(vertices[i].position);
		glm::vec3 p = radius * n;

		vertices[i].position = p;
		vertices[i].normal = n;

		float theta = atan2f(vertices[i].position.z, vertices[i].position.x);

		if (theta < 0.01f)
			theta += glm::pi<float>()/2;

		float phi = acosf(vertices[i].position.y / radius);

		vertices[i].tex.x = theta / glm::pi<float>() / 2;
		vertices[i].tex.y = phi / glm::pi<float>();

		// Partial derivative of P with respect to theta
		vertices[i].tangent.x = -radius * sinf(phi)*sinf(theta);
		vertices[i].tangent.y = 0.0f;
		vertices[i].tangent.z = +radius * sinf(phi)*cosf(theta);

		glm::vec3 T = vertices[i].tangent;
		vertices[i].tangent =  glm::normalize(T);
	}

	return Mesh(instance, vertices, indices, shininess);
}

Mesh Mesh::CreateSphere(DrawingInstance& instance, float radius, unsigned int slices, unsigned int stacks, glm::vec3 color, float shininess)
{
	// https://github.com/jjuiddong/Introduction-to-3D-Game-Programming-With-DirectX11/blob/master/Common/GeometryGenerator.h
	// https://github.com/jjuiddong/Introduction-to-3D-Game-Programming-With-DirectX11/blob/master/Common/GeometryGenerator.cpp
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Vertex TopVertex;
	Vertex BottomVertex;

	TopVertex.position = glm::vec3(0.0f, -radius, 0.0f);
	TopVertex.color = color;
	TopVertex.normal = glm::vec3(0.0f, -1.0f, 0.0);
	TopVertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
	TopVertex.tex = glm::vec2(0.0f, 0.0f);

	BottomVertex.position = glm::vec3(0.0f, radius, 0.0f);
	BottomVertex.color = color;
	BottomVertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
	BottomVertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
	BottomVertex.tex = glm::vec2(0.0f, 1.0f);

	vertices.push_back(TopVertex);

	float phiStep = glm::pi<float>() / stacks;
	float thetaStep = 2.0f*glm::pi<float>() / slices;

	for (unsigned int i = 1; i <= stacks; ++i)
	{
		float phi = i * phiStep;

		for (unsigned int j = 0; j <= slices; j++)
		{
			float theta = j * thetaStep;

			Vertex v;
			
			// spherical coordinates to cartesian
			v.position.x = radius * sinf(phi)*cosf(theta);
			v.position.y = -(radius * cosf(phi));
			v.position.z = (radius * sinf(phi) * sinf(theta));

			v.tangent.x = -radius * sinf(phi)*sinf(theta);
			v.tangent.y = 0.0f;
			v.tangent.z = +radius * sinf(phi)*cosf(theta);

			v.tangent = glm::normalize(v.tangent);
			v.normal = glm::normalize(v.position);

			v.tex.x = theta / glm::pi<float>()/2;
			v.tex.y = phi / glm::pi<float>();

			v.color = color;
			vertices.push_back(v);
		}
	}

	vertices.push_back(BottomVertex);

	// insert indices for top Stack
	for (unsigned int i = 1; i <= slices; ++i)
	{
		indices.push_back(0);
		indices.push_back(i+1);
		indices.push_back(i);
	}

	// insert indices for inner Stacks
	unsigned int baseIndex = 1;
	unsigned int ringVertexCount = slices + 1;
	for (unsigned int i = 0; i < stacks; ++i)
	{
		for (unsigned int j = 0; j < slices; ++j)
		{
			indices.push_back(baseIndex + i*ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i+1)*ringVertexCount + j);

			indices.push_back(baseIndex + (i+1)*ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1); 
			indices.push_back(baseIndex + (i+1) * ringVertexCount + j + 1);
		}
	}

	// insert indices for bottom stack
	unsigned int southPoleIndex = (unsigned int)vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;

	for (unsigned int i = 0; i < slices; ++i)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex+i);
		indices.push_back(baseIndex + i + 1);
	}

	return Mesh(instance, vertices, indices, shininess);
}

Mesh Mesh::CreatePlane(DrawingInstance& instance, float width, float depth, glm::vec3 color, float shininess)
{
	width = width * 0.5f;
	depth = depth * 0.5f;

	const std::vector<Vertex> vertices = {
				{ { -width, -depth, 0.0f },color, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, {-1.0, 0.0f, 0.0f} },
				{ { width, -depth, 0.0f },color, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, {-1.0, 0.0f, 0.0f} },
				{ { width, depth, 0.0f },color, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, {-1.0, 0.0f, 0.0f} },
				{ { -width, depth, 0.0f },color, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, {-1.0, 0.0f, 0.0f} },
	};

	const std::vector<uint32_t> indices = {
		0, 1, 2, 
		2, 3, 0
	};

	return Mesh(instance, vertices, indices, shininess);
}

Mesh Mesh::CreateGrid(DrawingInstance& instance, float width, float depth, unsigned int m, unsigned int n, glm::vec3 color, float lineWidth)
{
	// 3D Game Programming with DirectX 12 - P. 302ff

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	unsigned int vertexCount = m * n;
	unsigned int faceCount = (m - 1)*(n - 1) * 2;

	// Create vertices
	float halfWidth = width * 0.5f;
	float halfDepth = depth * 0.5f;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	vertices.resize(vertexCount);
	for (unsigned int i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (unsigned int j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			vertices[i*n + j].position = glm::vec3(x, z, 0.0f);
			vertices[i*n + j].color = color;
			vertices[i*n + j].normal = glm::vec3(0.0f, 0.0f, 1.0f);
			vertices[i*n + j].tangent = glm::vec3(1.0f, 0.0f, 0.0f);
			vertices[i*n + j].tex = glm::vec2(j*du, i*dv);
		}
	}

	// create Indizes
	indices.resize(faceCount * 4);

	unsigned int k = 0;
	for (unsigned int i = 0; i < m - 1; ++i)
	{
		for (unsigned int j = 0; j < n - 1; ++j)
		{

			indices[k + 0] = i * n + j;
			indices[k + 1] = i * n + (j + 1);
			indices[k + 2] = i * n + (j + 1);
			indices[k + 3] = (i + 1) * n + j + 1;

			indices[k + 4] = (i + 1) * n + j + 1;
			indices[k + 5] = (i + 1) * n + j;
			indices[k + 6] = (i + 1) * n + j;
			indices[k + 7] = i * n + j;

			k += 8;
		}
	}
	return Mesh(instance, vertices, indices, 16.0f, Invision::POLYGON_MODE_LINE, Invision::PRIMITIVE_TOPOLOGY_LINE_LIST, lineWidth);
}

void Mesh::Subdivide(std::vector<Vertex> &vertizes, std::vector<uint32_t> &indizes)
{
	// 3D Game Programming with DirectX 12 - P. 280f 

	// Save a copy of the input geometry.
	std::vector<Vertex> vertizesCopy = vertizes;
	std::vector<uint32_t> indizesCopy = indizes;


	vertizes.resize(0);
	indizes.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	uint32 numTris = (uint32)indizesCopy.size() / 3;
	for (uint32 i = 0; i < numTris; ++i)
	{
		Vertex v0 = vertizesCopy[indizesCopy[i * 3 + 0]];
		Vertex v1 = vertizesCopy[indizesCopy[i * 3 + 1]];
		Vertex v2 = vertizesCopy[indizesCopy[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		Vertex m0 = MidPoint(v0, v1);
		Vertex m1 = MidPoint(v1, v2);
		Vertex m2 = MidPoint(v0, v2);

		//
		// Add new geometry.
		//

		vertizes.push_back(v0); // 0
		vertizes.push_back(v1); // 1
		vertizes.push_back(v2); // 2
		vertizes.push_back(m0); // 3
		vertizes.push_back(m1); // 4
		vertizes.push_back(m2); // 5

		indizes.push_back(i * 6 + 0);
		indizes.push_back(i * 6 + 3);
		indizes.push_back(i * 6 + 5);

		indizes.push_back(i * 6 + 3);
		indizes.push_back(i * 6 + 4);
		indizes.push_back(i * 6 + 5);

		indizes.push_back(i * 6 + 5);
		indizes.push_back(i * 6 + 4);
		indizes.push_back(i * 6 + 2);

		indizes.push_back(i * 6 + 3);
		indizes.push_back(i * 6 + 1);
		indizes.push_back(i * 6 + 4);
	}
}

Vertex Mesh::MidPoint(const Vertex& v0, const Vertex& v1)
{
	// 3D Game Programming with DirectX 12 - P. 280f 

	glm::vec3 p0 = v0.position;
	glm::vec3 p1 = v1.position;

	glm::vec3 pos = 0.5f*(p0 + p1);



	Vertex v;
	v.position = pos;
	v.color = glm::normalize(0.5f*(v0.color + v1.color));
	v.normal = glm::normalize(0.5f*(v0.normal + v1.normal));
	v.tangent = glm::normalize(0.5f*(v0.tangent + v1.tangent));
	v.tex = glm::vec2( 0.5f*(v0.tex + v1.tex));

	return v;
}

/*std::shared_ptr <Invision::IUniformBuffer> Mesh::GetUniformBuffer()
{
	return uniformBuffer;
}*/

std::shared_ptr<Invision::IPipeline> Mesh::GetPipeline()
{
	return pipeline;
}

std::shared_ptr<Invision::IPipeline> Mesh::GetShadowPipeline()
{
	return mShadowPipeline;
}


std::shared_ptr<Invision::IPipeline> Mesh::GetGeomPipeline()
{
	return geomPipeline;
}

bool Mesh::IsIndexed()
{
	return isIndexed;
}