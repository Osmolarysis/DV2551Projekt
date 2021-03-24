#include "CubeState.h"
#include "..\..\Renderer\Renderer.h"
#include "..\..\Utility\Timer.h"
#include <iostream>
#include "..\..\DXUtility\DXUtility.h"
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
		//Update camera
		m_camera->update();

		//Update gif animation		
		m_scene[0]->getMesh(0)->getTexture()->updateAnimation(Renderer::getInstance()->getSwapChain()->GetCurrentBackBufferIndex(), Timer::getInstance()->getDt()); //R£££ Warning

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
	ID3D12RootSignature* rootSignature = Renderer::getInstance()->getRootSignature();
	ID3D12DescriptorHeap* cbDescriptorHeap[2] = { nullptr, nullptr };
	cbDescriptorHeap[0] = Renderer::getInstance()->getCBDescriptorHeap(0);
	cbDescriptorHeap[1] = Renderer::getInstance()->getCBDescriptorHeap(1);
	ID3D12GraphicsCommandList* commandList[2] = { nullptr, nullptr };
	commandList[0] = Renderer::getInstance()->getComputeCommandList(0);
	commandList[1] = Renderer::getInstance()->getComputeCommandList(1);
	UINT64 fenceValue = 0;
	int bbIndex = 0;

	//Wait for signal
	fence->SetEventOnCompletion(fenceValue + 1, handle);
	WaitForSingleObject(handle, INFINITE);

	while (m_computeThread.isActive) {
		//Initial work
		commandList[bbIndex]->SetComputeRootSignature(rootSignature);
		commandList[bbIndex]->SetPipelineState(m_computeStateObject.Get());
		commandList[bbIndex]->SetDescriptorHeaps(1, &cbDescriptorHeap[bbIndex]);
		commandList[bbIndex]->SetComputeRootDescriptorTable(0, cbDescriptorHeap[bbIndex]->GetGPUDescriptorHandleForHeapStart());
		commandList[bbIndex]->SetComputeRootUnorderedAccessView(1, m_ACBuffer[0]->GetGPUVirtualAddress());
		commandList[bbIndex]->SetComputeRootUnorderedAccessView(2, m_ACBuffer[1]->GetGPUVirtualAddress());

		//Thread work
		commandList[bbIndex]->Dispatch(1, 1, 1);

		//Thread handling
		bbIndex = 1 - bbIndex;
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
	v[8] = VertexBuffer::Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[9] = VertexBuffer::Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
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
	m_copyThread.isActive = false;
	if (m_copyThread.m_thread != nullptr)
	{
		Renderer::getInstance()->getCopyFence()->Signal(Renderer::getInstance()->getCopyValue() + 1);
		m_copyThread.m_thread->join();
		delete m_copyThread.m_thread;
	}
	m_copyThread.m_mutex.unlock();

	m_computeThread.m_mutex.lock();
	m_computeThread.isActive = false;
	if (m_computeThread.m_thread != nullptr)
	{
		Renderer::getInstance()->getComputeFence()->Signal(Renderer::getInstance()->getComputeValue() + 1);
		m_computeThread.m_thread->join();
		delete m_computeThread.m_thread;
	}
	m_computeThread.m_mutex.unlock();


	m_directThread.m_mutex.lock();
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
	Renderer* renderer = Renderer::getInstance();

	m_camera = std::make_unique<Camera>(true);

	// create meshGroup
	LPCWSTR shaderFiles[] = { L"Source/Shaders/VertexShader.hlsl", L"Source/Shaders/PixelShader.hlsl" };
	UINT cbufferSize = sizeof(XMMATRIX);
	UINT cbufferLocation = 1;
	m_scene.push_back(std::make_unique<MeshGroup>(shaderFiles, cbufferSize, cbufferLocation));

	//Create triangle (later cube) - from box example

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(createBox(1, 1, 1), "Assets/Images/frame_", ".png", 186);

	// Add VertexBuffer (or Mesh) to the MeshGroup. (Mesh transform default to (0,0,0))
	m_scene[0]->addMesh(mesh);

	//Rotation Buffer
	ID3D12Device* device = renderer->getDevice();
	//device->CreateCommittedResource();

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

	//Transformation

	std::wstring initialisationShader = L"Source/Shaders/CS_Initialisation.hlsl";
	std::wstring rotationShader = L"Source/Shaders/CS_CubeRotator.hlsl";
	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors = nullptr;

	//	Compile shader
	HRESULT hr = D3DCompileFromFile(
		rotationShader.c_str(),
		nullptr,
		nullptr,
		"main",
		"cs_5_0",
		0,
		0,
		byteCode.GetAddressOf(),
		errors.GetAddressOf()
	);
	if (errors != nullptr || hr != S_OK)
	{
		printf("Error compiling compute shader\n");
		OutputDebugStringA((char*)errors->GetBufferPointer());
		exit(-1);
	}

	D3D12_COMPUTE_PIPELINE_STATE_DESC cpsd = {};
	cpsd.pRootSignature = renderer->getRootSignature();
	cpsd.CS.pShaderBytecode = reinterpret_cast<BYTE*>(byteCode->GetBufferPointer());
	cpsd.CS.BytecodeLength = byteCode->GetBufferSize();
	cpsd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	cpsd.NodeMask = 0;

	hr = renderer->getDevice()->CreateComputePipelineState(&cpsd, IID_PPV_ARGS(m_computeStateObject.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating compute pipeline state object\n");
		exit(-1);
	}
	m_computeStateObject->SetName(L"Compute pipeline state object");

	for (size_t i = 0; i < 2; i++)
	{
		std::wstring name = L"Append & consume buffer ";
		name.append(std::to_wstring(i));
		m_ACBuffer[i] = CreateDefaultBuffer(renderer->getCopyCommandList(), m_transformationMatrix, sizeof(m_transformationMatrix), m_ACHeap[i], name.c_str(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}

void CubeState::update()
{
	//Update the rotation matrix
	static double timer = 0;
	timer += Timer::getInstance()->getDt();
	int axis = (int)(timer * 0.5) % 3;
	m_scene[0]->getMesh(0)->rotate((float)Timer::getInstance()->getDt(), axis);
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
