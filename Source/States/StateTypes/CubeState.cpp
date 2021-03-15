#include "CubeState.h"
#include "..\..\Renderer\Renderer.h"
#include "..\..\Utility\Timer.h"
#include <iostream>
using namespace DirectX;

void CubeState::copyRecord()
{
	while (m_copyThread.isActive) {
		while (m_copyThread.isRunning) {
			
			//Thread work
			

			//Thread handling
			m_copyThread.m_mutex.lock();
			m_copyThread.isRunning = false;
			m_copyThread.m_mutex.unlock();
		}
	}
}

void CubeState::computeRecord()
{
	while (m_computeThread.isActive) {
		while (m_computeThread.isRunning) {

			//Thread work


			//Thread handling
			m_computeThread.m_mutex.lock();
			m_computeThread.isRunning = false;
			m_computeThread.m_mutex.unlock();
		}
	}
}

void CubeState::directRecord()
{
	while (m_directThread.isActive) {
		while (m_directThread.isRunning) {

			//Thread work
			for (auto& meshG : m_scene)
			{
				meshG->drawAll();
			}

			//Thread handling
			m_directThread.m_mutex.lock();
			m_directThread.isRunning = false;
			m_directThread.m_mutex.unlock();
		}
	}
}

CubeState::CubeState()
{
	printf("Constructing cubeState...\n"); //For debugging, remove when implementing

}

CubeState::~CubeState()
{
	printf("Destroying cubeState...\n");

	//Multithreads
	m_copyThread.m_mutex.lock();
	m_copyThread.isRunning = false;
	m_copyThread.isActive = false;
	m_copyThread.m_thread->join();
	delete m_copyThread.m_thread;
	m_copyThread.m_mutex.unlock();

	m_computeThread.m_mutex.lock();
	m_computeThread.isRunning = false;
	m_computeThread.isActive = false;
	m_computeThread.m_thread->join();
	delete m_computeThread.m_thread;
	m_computeThread.m_mutex.unlock();

	m_directThread.m_mutex.lock();
	m_directThread.isRunning = false;
	m_directThread.isActive = false;
	m_directThread.m_thread->join();
	delete m_directThread.m_thread;
	m_directThread.m_mutex.unlock();
}

void CubeState::initialise()
{
	printf("Initialising cubeState...\n"); //For debugging, remove when implementing

	m_camera = std::make_unique<Camera>();

	// create meshGroup
	LPCWSTR shaderFiles[] = { L"Source/Shaders/VertexShader.hlsl", L"Source/Shaders/PixelShader.hlsl" };
	UINT cbufferSize = sizeof(XMMATRIX);
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

	//Multithread
	m_copyThread.m_mutex.lock();
	m_copyThread.m_thread = new std::thread([this] {copyRecord(); });
	m_copyThread.m_mutex.unlock();

	m_computeThread.m_mutex.lock();
	m_computeThread.m_thread = new std::thread([this] {computeRecord(); });
	m_computeThread.m_mutex.unlock();

	m_directThread.m_mutex.lock();
	m_directThread.m_thread = new std::thread([this] {directRecord(); });
	m_directThread.m_mutex.unlock();
}

void CubeState::update()
{
	//Update the rotation matrix
	static double timer = 0;
	timer += Timer::getInstance()->getDt();
	int axis = (int)(timer * 0.5) % 3;
	m_scene[0]->getMesh(0)->rotate((float)Timer::getInstance()->getDt(), axis);

	//Update camera
	m_camera->update();
}

void CubeState::record()
{
	//thread is running = true
	m_copyThread.m_mutex.lock();
	m_copyThread.isRunning = true;
	m_copyThread.m_mutex.unlock();

	m_computeThread.m_mutex.lock();
	m_computeThread.isRunning = true;
	m_computeThread.m_mutex.unlock();

	m_directThread.m_mutex.lock();
	m_directThread.isRunning = true;
	m_directThread.m_mutex.unlock();

	int loops = 0;

	//Signal when all threads are done
	while (m_directThread.isRunning || m_computeThread.isRunning || m_copyThread.isRunning)
	{
		loops++;
		printf("Loop: %i\n", loops);
	}
}

void CubeState::executeList()
{

}
