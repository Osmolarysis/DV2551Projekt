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

std::shared_ptr<VertexBuffer> CubeState::createBox(float width, float height, float depth)
{
	VertexBuffer::Vertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = VertexBuffer::Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[1] = VertexBuffer::Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[2] = VertexBuffer::Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[3] = VertexBuffer::Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = VertexBuffer::Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[5] = VertexBuffer::Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[6] = VertexBuffer::Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[7] = VertexBuffer::Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8]  = VertexBuffer::Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[9]  = VertexBuffer::Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[10] = VertexBuffer::Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[11] = VertexBuffer::Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = VertexBuffer::Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[13] = VertexBuffer::Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[14] = VertexBuffer::Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[15] = VertexBuffer::Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = VertexBuffer::Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[17] = VertexBuffer::Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[18] = VertexBuffer::Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[19] = VertexBuffer::Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = VertexBuffer::Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[21] = VertexBuffer::Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[22] = VertexBuffer::Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[23] = VertexBuffer::Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);


	UINT16 i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	int size = sizeof(v);
	int indexSize = sizeof(i);
	std::shared_ptr<VertexBuffer> vertBuf = std::make_shared<VertexBuffer>();
	vertBuf->setData(v, size, i, indexSize);

	return vertBuf;
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

	std::shared_ptr<VertexBuffer> vertBuf = createBox(1,1,1);

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
