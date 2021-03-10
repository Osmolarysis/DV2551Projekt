#include "CubeState.h"
#include "..\..\Renderer\Renderer.h"
#include <iostream>

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

	//Create triangle (later cube)
	VertexBuffer::Vertex meshVertices[] =
	{
		{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	};
	int size = sizeof(meshVertices);
	std::shared_ptr<VertexBuffer> vertBuf = std::make_shared<VertexBuffer>(size);
	vertBuf->setData(meshVertices); // note to self: offset and nrofVertices here is redundant if vertex struct is static

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
