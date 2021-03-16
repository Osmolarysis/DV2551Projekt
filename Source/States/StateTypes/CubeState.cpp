#include "CubeState.h"
#include "..\..\Renderer\Renderer.h"
#include "..\..\Utility\Timer.h"
#include <iostream>
using namespace DirectX;

void CubeState::copyRecord()
{
	ID3D12Fence1* fence = Renderer::getInstance()->getCopyFence();
	HANDLE handle = Renderer::getInstance()->getCopyThreadHandle();
	UINT64 fenceValue = 0;

	//Wait for signal
	fence->SetEventOnCompletion(fenceValue + 1, handle);
	WaitForSingleObject(handle, INFINITE);

	while (m_copyThread.isActive) {
		//Thread work

		//Thread handling
		fenceValue = Renderer::getInstance()->incAndGetCopyValue();
		fence->Signal(fenceValue); //Done

		//Wait for signal
		fence->SetEventOnCompletion(fenceValue + 1, handle);
		WaitForSingleObject(handle, INFINITE);
	}
}

void CubeState::computeRecord()
{
	ID3D12Fence1* fence = Renderer::getInstance()->getComputeFence();
	HANDLE handle = Renderer::getInstance()->getComputeThreadHandle();
	UINT64 fenceValue = 0;

	//Wait for signal
	fence->SetEventOnCompletion(fenceValue + 1, handle);
	WaitForSingleObject(handle, INFINITE);

	while (m_computeThread.isActive) {
		//Thread work

		//Thread handling
		fenceValue = Renderer::getInstance()->incAndGetComputeValue();
		fence->Signal(fenceValue); //Done

		//Wait for signal
		fence->SetEventOnCompletion(fenceValue + 1, handle);
		WaitForSingleObject(handle, INFINITE);
	}
}

void CubeState::directRecord()
{
	ID3D12Fence1* fence = Renderer::getInstance()->getDirectFence();
	HANDLE handle = Renderer::getInstance()->getDirectThreadHandle();
	UINT64 fenceValue = 0;

	HRESULT hr;

	//Wait for signal
	hr = fence->SetEventOnCompletion(fenceValue + 1, handle);
	WaitForSingleObject(handle, INFINITE);

	while (m_directThread.isActive) {
		//Thread work
		for (auto& meshG : m_scene)
		{
			meshG->drawAll();
		}

		//Thread handling
		fenceValue = Renderer::getInstance()->incAndGetDirectValue();
		fence->Signal(fenceValue); //Done

		//Wait for signal
		hr = fence->SetEventOnCompletion(fenceValue + 1, handle);
		WaitForSingleObject(handle, INFINITE);
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
	if (m_copyThread.m_thread != nullptr)
	{
		Renderer::getInstance()->getCopyFence()->Signal(Renderer::getInstance()->getCopyValue() + 1);
		m_copyThread.m_thread->join();
		delete m_copyThread.m_thread;
	}
	m_copyThread.m_mutex.unlock();

	m_computeThread.m_mutex.lock();
	m_computeThread.isRunning = false;
	m_computeThread.isActive = false;
	if (m_computeThread.m_thread != nullptr)
	{
		Renderer::getInstance()->getComputeFence()->Signal(Renderer::getInstance()->getComputeValue() + 1);
		m_computeThread.m_thread->join();
		delete m_computeThread.m_thread;
	}
	m_computeThread.m_mutex.unlock();


	m_directThread.m_mutex.lock();
	m_directThread.isRunning = false;
	m_directThread.isActive = false;
	if (m_directThread.m_thread != nullptr)
	{
		Renderer::getInstance()->getDirectFence()->Signal(Renderer::getInstance()->getDirectValue() + 1);
		m_directThread.m_thread->join();
		delete m_directThread.m_thread;
	}
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
	std::shared_ptr<VertexBuffer> vertBuf = std::make_shared<VertexBuffer>();
	vertBuf->setData(meshVertices, size, indices, indexSize);

	// Add VertexBuffer (or Mesh) to the MeshGroup. (Mesh transform default to (0,0,0))
	m_scene[0]->addMesh(vertBuf);

	////Floor
	//VertexBuffer::Vertex floorVertices[] =
	//{
	//	{ XMFLOAT3(-10.0f, -3.0f, -10.0f), XMFLOAT4(0.6f,0.6f,0.6f,1) },
	//	{ XMFLOAT3(-10.0f, -3.0f, +10.0f), XMFLOAT4(0.6f,0.6f,0.6f,1) },
	//	{ XMFLOAT3(+10.0f, -3.0f, +10.0f), XMFLOAT4(0.6f,0.6f,0.6f,1) },
	//	{ XMFLOAT3(+10.0f, -3.0f, -10.0f), XMFLOAT4(0.6f,0.6f,0.6f,1) }
	//};

	//UINT16 floorIndices[] =
	//{
	//	0, 1, 2,
	//	0, 2, 3,
	//};

	//int floorSize = sizeof(floorVertices);
	//int florrIndexSize = sizeof(floorIndices);
	//std::shared_ptr<VertexBuffer> floorvertBuf = std::make_shared<VertexBuffer>();
	//floorvertBuf->setData(floorVertices, floorSize, floorIndices, florrIndexSize);

	//// Add VertexBuffer (or Mesh) to the MeshGroup. (Mesh transform default to (0,0,0))
	//m_scene[0]->addMesh(floorvertBuf);

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
	Renderer* renderer = Renderer::getInstance();

	//Set threads to run = true
	UINT64 copyFenceValue = renderer->incAndGetCopyValue();
	renderer->getCopyFence()->Signal(copyFenceValue); //Ready or "Run"
	renderer->getCopyFence()->SetEventOnCompletion(copyFenceValue + 1, renderer->getCopyHandle());

	UINT64 computeFenceValue = renderer->incAndGetComputeValue();
	renderer->getComputeFence()->Signal(computeFenceValue); //Ready or "Run"
	renderer->getComputeFence()->SetEventOnCompletion(computeFenceValue + 1, renderer->getComputeHandle());

	UINT64 directFenceValue = renderer->incAndGetDirectValue();
	renderer->getDirectFence()->Signal(directFenceValue); //Ready or "Run"
	renderer->getDirectFence()->SetEventOnCompletion(directFenceValue + 1, renderer->getDirectHandle());
}

void CubeState::executeList()
{

}
