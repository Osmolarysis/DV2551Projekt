#include "CubeState.h"
#include "..\..\Renderer\Renderer.h"
#include <iostream>
using namespace DirectX;

CubeState::CubeState()
{
	printf("Constructing cubeState...\n"); //For debugging, remove when implementing
}

CubeState::~CubeState()
{
	printf("Destroying cubeState...\n");
}

void CubeState::initialise()
{
	printf("Initialising cubeState...\n"); //For debugging, remove when implementing

	m_camera = std::make_unique<Camera>();
	
	// create meshGroup
	LPCWSTR shaderFiles[] = { L"Source/Shaders/VertexShader.hlsl", L"Source/Shaders/PixelShader.hlsl" };
	UINT cbufferSize = sizeof(float) * 4;
	UINT cbufferLocation = 1;
	m_scene.push_back(std::make_unique<MeshGroup>(shaderFiles, cbufferSize, cbufferLocation));

	//Create triangle (later cube) - from box example
	VertexBuffer::Vertex meshVertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1,1,1,1) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0,0,0,1) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(1,0,0,1) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(0,1,0,1) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(0,0,1,1) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(1,1,0,1) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(0,1,1,1) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1,0,1,1) }
	};

	UINT16 indices[] =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	int size = sizeof(meshVertices);
	int indexSize = sizeof(indices);
	std::shared_ptr<VertexBuffer> vertBuf = std::make_shared<VertexBuffer>(size, indexSize);
	vertBuf->setData(meshVertices, indices); // note to self: offset and nrofVertices here is redundant if vertex struct is static

	// Add VertexBuffer (or Mesh) to the MeshGroup. (Mesh transform default to (0,0,0))
	m_scene[0]->addMesh(vertBuf);

}

void CubeState::update()
{
	//Update the rotation matrix

	//Update camera
	m_camera->update();
}

void CubeState::record()
{
	for (auto& meshG : m_scene)
	{
		meshG->drawAll();
	}
}

void CubeState::executeList()
{

}
