#include "Renderer.h"
#include "..\Utility\Timer.h"
#include "..\Utility\Input.h"


Renderer Renderer::m_this(1280, 720);

Renderer::Renderer(int width, int height)
{
	m_screenWidth = width;
	m_screenHeight = height;

	//Set debug mode
	if (!createDebugMode()) {
		printf("error setting debug mode\n");
		exit(-1);
	}

	//Create window
	if (!createWindow()) {
		printf("error creating window\n");
		exit(-1);
	}

	//Create device
	if (!createDevice()) {
		printf("error creating device\n");
		exit(-1);
	}

	//Create command queue and lists
	if (!createCommandQueues()) {
		printf("error creating command queues\n");
		exit(-1);
	}

	//create swap chain
	if (!createSwapChain()) {
		printf("error creating swap chain\n");
		exit(-1);
	}

	//create fence and event handler
	if (!createFenceAndEventHandle()) {
		printf("error creating fence and event handler\n");
		exit(-1);
	}

	//create descriptor heaps
	if (!createDescriptorHeap()) {
		printf("error creating descriptor heap\n");
		exit(-1);
	}

	//create render target
	if (!createRenderTargets()) {
		printf("error creating render target\n");
		exit(-1);
	}

	//create depth stencil
	if (!createDepthStencil()) {
		printf("error creating depth stencil\n");
		exit(-1);
	}

	//create view port and scissor rect
	if (!createViewportAndScissorRect()) {
		printf("error creating view port and scissor rect\n");
		exit(-1);
	}

	//create root signature
	if (!createRootSignature()) {
		printf("error creating root signature\n");
		exit(-1);
	}
}

Renderer::~Renderer()
{
	//Debug
	SafeRelease(m_debugController.GetAddressOf());

	//Window
	CloseWindow(m_handle);

	//Device
	SafeRelease(m_factory.GetAddressOf());
	SafeRelease(m_device.GetAddressOf());

	//Root signature
	SafeRelease(m_rootSignature.GetAddressOf());

	//Swapchain
	SafeRelease(m_swapChain.GetAddressOf());

	//Render Target
	SafeRelease(m_renderTargetHeap.GetAddressOf());
	for (size_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		SafeRelease(m_renderTargets[i].GetAddressOf());
		SafeRelease(m_cbDescriptorHeaps[i].GetAddressOf());
	}

	//Direct queue/list/allocator
	SafeRelease(m_directQueue.GetAddressOf());
	for (size_t i = 0; i < NUM_COMMANDLISTS; i++)
	{
		SafeRelease(m_directAllocator[i].GetAddressOf());
		SafeRelease(m_graphicsDirectList[i].GetAddressOf());
	}

	//Copy queue/list/allocator
	SafeRelease(m_copyQueue.GetAddressOf());
	for (size_t i = 0; i < NUM_COMMANDLISTS; i++)
	{
		SafeRelease(m_copyAllocator[i].GetAddressOf());
		SafeRelease(m_graphicsCopyList[i].GetAddressOf());
	}

	//Compute queue/list/allocator
	SafeRelease(m_computeQueue.GetAddressOf());
	for (size_t i = 0; i < NUM_COMMANDLISTS; i++)
	{
		SafeRelease(m_computeAllocator[i].GetAddressOf());
		SafeRelease(m_graphicsComputeList[i].GetAddressOf());
	}

	//Fence and event handle
	SafeRelease(m_fence.GetAddressOf());
}

bool Renderer::createDepthStencil()
{
	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = m_screenWidth;
	resourceDesc.Height = m_screenHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D32_FLOAT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	hp.CreationNodeMask = 1; //used when multi-gpu (we don't do that here)
	hp.VisibleNodeMask = 1; //used when multi-gpu (we don't do that here)
	hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	m_depthBufferDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	for (size_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		HRESULT hr = m_device.Get()->CreateCommittedResource(
			&hp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(m_depthStencilBuffer[i].GetAddressOf()));

		if (hr != S_OK) {
			return false;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_dbDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart();
		cdh.ptr += (SIZE_T)m_depthBufferDescriptorSize * (SIZE_T)i;

		m_device.Get()->CreateDepthStencilView(m_depthStencilBuffer[i].Get(), &depthStencilDesc, cdh);
	}


	return true;
}

Renderer* Renderer::getInstance()
{
	return &m_this;
}

ID3D12Device8* Renderer::getDevice()
{
	return m_device.Get();
}

ID3D12GraphicsCommandList* Renderer::getDirectCommandList()
{
	int index = m_swapChain->GetCurrentBackBufferIndex();
	return m_graphicsDirectList[index].Get();
}

ID3D12GraphicsCommandList* Renderer::getDirectCommandList(int bufferIndex)
{
	return m_graphicsDirectList[bufferIndex].Get();
}

void Renderer::closeCommandLists()
{
	HRESULT hr;

	for (int i = 0; i < NUM_COMMANDLISTS; i++) {
		hr = m_graphicsCopyList[i].Get()->Close();
		if (hr != S_OK) {
			printf("Error closing copy list at close");
			exit(-1);
		}

		hr = m_graphicsComputeList[i].Get()->Close();
		if (hr != S_OK) {
			printf("Error closing compute list at close");
			exit(-1);
		}

		hr = m_graphicsDirectList[i].Get()->Close();
		if (hr != S_OK) {
			printf("Error closing direct list at close");
			exit(-1);
		}
	}

	//Should maybe do some fencing here.
	ID3D12CommandList* copyListsToExecute[] = { m_graphicsCopyList[0].Get(), m_graphicsCopyList[1].Get() };
	m_copyQueue->ExecuteCommandLists(ARRAYSIZE(copyListsToExecute), copyListsToExecute);

	ID3D12CommandList* computeListsToExecute[] = { m_graphicsComputeList[0].Get(), m_graphicsComputeList[1].Get() };
	m_computeQueue->ExecuteCommandLists(ARRAYSIZE(computeListsToExecute), computeListsToExecute);

	ID3D12CommandList* directListsToExecute[] = { m_graphicsDirectList[0].Get(), m_graphicsDirectList[1].Get() };
	m_directQueue->ExecuteCommandLists(ARRAYSIZE(directListsToExecute), directListsToExecute);

	waitForGPU();
}

ID3D12RootSignature* Renderer::getRootSignature()
{
	return m_rootSignature.Get();
}

unsigned int Renderer::getScreenWidth() const
{
	return m_screenWidth;
}

IDXGISwapChain4* Renderer::getSwapChain()
{
	return m_swapChain.Get();
}

HWND Renderer::getWindowHandle() 
{ 
	return m_handle; 
}

unsigned int Renderer::getScreenHeight() const
{
	return m_screenHeight;
}

void Renderer::beginFrame()
{

	setWindowTitle(L"Projekt");

	UINT backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	UINT64 lastFinishedQueue = m_fence.Get()->GetCompletedValue(); //Number of last finished queue

	//Wait
	/*while (m_fenceValue - lastFinishedQueue >= 2) {
		lastFinishedQueue = m_fence.Get()->GetCompletedValue();
	}*/
	if (m_fenceValue - lastFinishedQueue >= 2) {
		m_fence.Get()->SetEventOnCompletion(m_fenceValue - UINT64(1), m_eventHandle);
		WaitForSingleObject(m_eventHandle, INFINITE);
	}

	float clearColour[4] = { 0.3f, 0.3f, 0.0f, 1.0f };

	//Clear
	//Copy
	HRESULT hr = m_copyAllocator[backBufferIndex].Get()->Reset();
	if (hr != S_OK) {
		printf("Error reseting copy allocator %i\n", backBufferIndex);
		exit(-1);
	}

	hr = m_graphicsCopyList[backBufferIndex].Get()->Reset(m_copyAllocator[backBufferIndex].Get(), nullptr);
	if (hr != S_OK) {
		printf("Error reseting copy list %i\n", backBufferIndex);
		exit(-1);
	}

	////Compute
	//hr = m_computeAllocator[backBufferIndex].Get()->Reset();
	//if (hr != S_OK) {
	//	printf("Error reseting compute allocator %i\n", backBufferIndex);
	//	exit(-1);
	//}

	//hr = m_graphicsComputeList[backBufferIndex].Get()->Reset(m_computeAllocator[backBufferIndex].Get(), nullptr);
	//if (hr != S_OK) {
	//	printf("Error reseting compute list %i\n", backBufferIndex);
	//	exit(-1);
	//}

	//Direct
	hr = m_directAllocator[backBufferIndex].Get()->Reset();
	if (hr != S_OK) {
		printf("Error reseting direct allocator %i\n", backBufferIndex);
		exit(-1);
	}

	hr = m_graphicsDirectList[backBufferIndex].Get()->Reset(m_directAllocator[backBufferIndex].Get(), nullptr);
	if (hr != S_OK) {
		printf("Error reseting direct list %i\n", backBufferIndex);
		exit(-1);
	}


	//Set necessary states.
	m_graphicsDirectList[backBufferIndex].Get()->RSSetViewports(1, &m_viewPort);
	m_graphicsDirectList[backBufferIndex].Get()->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(
		m_graphicsDirectList[backBufferIndex].Get(),
		m_renderTargets[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);

	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetHeap.Get()->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += (SIZE_T)m_renderTargetDescriptorSize * (SIZE_T)backBufferIndex;

	m_graphicsDirectList[backBufferIndex].Get()->ClearRenderTargetView(cdh,
		clearColour, 0, nullptr);

	if (!dsvSetWrite[backBufferIndex]) { //TODO: Find a better way than this, maybe "gpu warm up" function
		SetResourceTransitionBarrier(
			m_graphicsDirectList[backBufferIndex].Get(),
			m_depthStencilBuffer[backBufferIndex].Get(),
			D3D12_RESOURCE_STATE_COMMON,		//state before
			D3D12_RESOURCE_STATE_DEPTH_WRITE	//state after
		);
		dsvSetWrite[backBufferIndex] = true;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DBcdh = m_dbDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart();
	DBcdh.ptr += (SIZE_T)m_depthBufferDescriptorSize * (SIZE_T)backBufferIndex;

	m_graphicsDirectList[backBufferIndex].Get()->ClearDepthStencilView(DBcdh, D3D12_CLEAR_FLAG_DEPTH /*| D3D12_CLEAR_FLAG_STENCIL*/, 1.0f, 0, 1, &m_scissorRect);

	// Specify the buffers we are going to render to. Correct render target?
	m_graphicsDirectList[backBufferIndex].Get()->OMSetRenderTargets(1, &cdh, true, &DBcdh);

	//Set root signature
	m_graphicsDirectList[backBufferIndex].Get()->SetGraphicsRootSignature(m_rootSignature.Get());

	m_graphicsDirectList[backBufferIndex].Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Set constant buffer descriptor heaps
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbDescriptorHeaps[backBufferIndex].Get() };
	m_graphicsDirectList[backBufferIndex].Get()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);
	m_graphicsDirectList[backBufferIndex].Get()->SetGraphicsRootDescriptorTable(0, m_cbDescriptorHeaps[backBufferIndex].Get()->GetGPUDescriptorHandleForHeapStart());

	//descriptorHeaps[0] = m_SRVDescriptorHeaps[backBufferIndex].Get();
	//m_graphicsDirectList[backBufferIndex].Get()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);
	//m_graphicsDirectList[backBufferIndex].Get()->SetGraphicsRootDescriptorTable(1, m_SRVDescriptorHeaps[backBufferIndex].Get()->GetGPUDescriptorHandleForHeapStart());

	//Set waiting criteria
	//m_renderingFence.Get()->Signal(101);
	//m_computeQueue.Get()->Wait(m_renderingFence.Get(), 102);
	m_directQueue.Get()->Wait(m_renderingFence.Get(), 102);
}

void Renderer::executeList()
{
	UINT backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	//Wait for Copy queue to finish recording

	WaitForSingleObject(m_copyHandle, INFINITE);

	//Execute Copy queue

	HRESULT hr = m_graphicsCopyList[backBufferIndex].Get()->Close();
	if (hr != S_OK) {
		printf("Error closing copy list %i\n", backBufferIndex);
		exit(-1);
	}

	ID3D12CommandList* listsToExecuteCopy[] = { m_graphicsCopyList[backBufferIndex].Get() };
	m_copyQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecuteCopy), listsToExecuteCopy);
	m_copyQueue->Signal(m_renderingFence.Get(), 102);


	//Wait for Compute queue to finish recording

	WaitForSingleObject(m_computeHandle, INFINITE);

	//Execute Compute queue
	//m_computeQueue->Signal(m_renderingFence.Get(), 201); //Starting
	
	//Work

	//m_computeQueue->Signal(m_renderingFence.Get(), 202); //Done

	//Wait for Direct queue to finish recording

	WaitForSingleObject(m_directHandle, INFINITE);

	//Execute Direct queue

	SetResourceTransitionBarrier(m_graphicsDirectList[backBufferIndex].Get(),
		m_renderTargets[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	hr = m_graphicsDirectList[backBufferIndex].Get()->Close();
	if (hr != S_OK) {
		printf("Error closing direct list %i\n", backBufferIndex);
		exit(-1);
	}

	m_directQueue->Signal(m_renderingFence.Get(), 301); //Direct Starting

	//Execute the commands!
	ID3D12CommandList* listsToExecute[] = { m_graphicsDirectList[backBufferIndex].Get() };
	m_directQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//m_directQueue->Signal(m_renderingFence.Get(), 302); //Direct finished

	m_fenceValue++;

	//Set finished rendering value
	m_directQueue.Get()->Signal(m_fence.Get(), m_fenceValue);
}

void Renderer::present()
{
	//Swap front and back buffers
	DXGI_PRESENT_PARAMETERS pp = {}; //Are these important?
	m_swapChain->Present1(0, 0, &pp);
}

void Renderer::SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	//Blatantly borrowed from Stefan or Fransisco
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}

void Renderer::setWindowTitle(std::wstring newTitle)
{
	Timer* timer = Timer::getInstance();
	double dt = timer->getDt();
	double avgFPS = timer->getAverageFPS(1000);
	std::wstring dt_str = std::to_wstring(dt);
	std::wstring avg_fps_str = std::to_wstring(avgFPS);
	m_windowTitle = newTitle + L" dt: " + dt_str + L"s, avg fps: " + avg_fps_str;

	SetWindowText(m_handle, m_windowTitle.c_str());
}

void Renderer::waitForGPU()
{
	//Wait for copy queue
	const UINT64 fenceCopy = m_fenceValue;
	m_copyQueue->Signal(m_fence.Get(), fenceCopy);
	m_fenceValue++;

	//Wait until copy queue is done.
	if (m_fence->GetCompletedValue() < fenceCopy)
	{
		m_fence->SetEventOnCompletion(fenceCopy, m_eventHandle);
		WaitForSingleObject(m_eventHandle, INFINITE);
	}

	//Wait for compute queue
	const UINT64 fenceCompute = m_fenceValue;
	m_computeQueue->Signal(m_fence.Get(), fenceCompute);
	m_fenceValue++;

	//Wait until compute queue is done.
	if (m_fence->GetCompletedValue() < fenceCompute)
	{
		m_fence->SetEventOnCompletion(fenceCompute, m_eventHandle);
		WaitForSingleObject(m_eventHandle, INFINITE);
	}

	const UINT64 fenceDirect = m_fenceValue;
	m_directQueue->Signal(m_fence.Get(), fenceDirect);
	m_fenceValue++;

	//Wait until direct queue is done.
	if (m_fence->GetCompletedValue() < fenceDirect)
	{
		m_fence->SetEventOnCompletion(fenceDirect, m_eventHandle);
		WaitForSingleObject(m_eventHandle, INFINITE);
	}
}

ID3D12DescriptorHeap* Renderer::getCBDescriptorHeap(UINT bufferIndex)
{
	return m_cbDescriptorHeaps[bufferIndex].Get();
}

ID3D12DescriptorHeap* Renderer::getSRVDescriptorHeap(UINT bufferIndex)
{
	return m_SRVDescriptorHeaps[bufferIndex].Get();
}

ID3D12Fence1* Renderer::getCopyFence()
{
	return m_copyFence.Get();
}

HANDLE Renderer::getCopyHandle()
{
	return m_copyHandle;
}

HANDLE Renderer::getCopyThreadHandle()
{
	return m_copyThreadHandle;
}

ID3D12GraphicsCommandList* Renderer::getCopyCommandList()
{
	int index = m_swapChain->GetCurrentBackBufferIndex();
	return m_graphicsCopyList[index].Get();
}

UINT64 Renderer::getCopyValue()
{
	return m_copyFenceValue;
}

UINT64 Renderer::incAndGetCopyValue()
{
	m_copyFenceValue++;
	return m_copyFenceValue;
}

ID3D12Fence1* Renderer::getComputeFence()
{
	return m_computeFence.Get();
}

UINT64 Renderer::getComputeValue()
{
	return m_computeFenceValue;
}

UINT64 Renderer::incAndGetComputeValue()
{
	m_computeFenceValue++;
	return m_computeFenceValue;
}

HANDLE Renderer::getComputeHandle()
{
	return m_computeHandle;
}

HANDLE Renderer::getComputeThreadHandle()
{
	return m_computeThreadHandle;
}

ID3D12GraphicsCommandList* Renderer::getComputeCommandList()
{
	int index = m_swapChain->GetCurrentBackBufferIndex();
	return m_graphicsComputeList[index].Get();
}

ID3D12Fence1* Renderer::getDirectFence()
{
	return m_directFence.Get();
}

UINT64 Renderer::getDirectValue()
{
	return m_directFenceValue;
}

UINT64 Renderer::incAndGetDirectValue()
{
	m_directFenceValue++;
	return m_directFenceValue;
}

HANDLE Renderer::getDirectHandle()
{
	return m_directHandle;
}

HANDLE Renderer::getDirectThreadHandle()
{
	return m_directThreadHandle;
}

void Renderer::setFence(int fence, int value)
{
	m_directQueue.Get()->Signal(m_fence.Get(), value);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool Renderer::createWindow()
{
	// Define window style
	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = m_windowTitle.c_str();
	RegisterClass(&wc);

	// Create the window
	m_handle = CreateWindow(m_windowTitle.c_str(), m_windowTitle.c_str(),
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, 0, m_screenWidth,
		m_screenHeight + 30, nullptr, nullptr, nullptr, nullptr);

	return true;
}

bool Renderer::createDevice() // DXR support is assumed... todo
{
	ComPtr<IDXGIAdapter1> adapter; //Want to go four, QueryInterface() is a solution.

	HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(m_factory.GetAddressOf()));
	if (hr != S_OK) {
		return false;
	}

	for (size_t i = 0;; i++)
	{
		adapter.Reset();
		if (DXGI_ERROR_NOT_FOUND == m_factory->EnumAdapters1((UINT)i, adapter.GetAddressOf())) {
			break; //We ran out of adapters
		}

		//Check if the adapter supports out feature level, but dont create
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr))) {
			break;
		}

		adapter->Release();
	}

	if (adapter.Get()) {
		hr = D3D12CreateDevice(
			adapter.Get(), //nullptr if default adapter
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(m_device.GetAddressOf())
		);
		if (hr != S_OK) {
			return false;
		}
	}
	else {
		// No adapter with level 12.1
		return false;
	}

	//adapter->Release(); //Does this automatically
	return true;
}

bool Renderer::createDebugMode()
{
	HMODULE mD3D12 = LoadLibrary(L"D3D12.dll");
	PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
	if (SUCCEEDED(f(IID_PPV_ARGS(m_debugController.GetAddressOf())))) {
		m_debugController->EnableDebugLayer();
		m_debugController->SetEnableGPUBasedValidation(true);
		return true;
	}
	else {
		return false;
	}
}

bool Renderer::createCommandQueues()
{
	//One of each type of queue (3 total), but six lists (possible to go 3 lists with 6 allocators, but hard appearently)
	//Start with grade D, 1 Direct queue and 2 lists
	if (!createDirectQueue()) {
		return false;
	}
	if (!createCopyQueue()) {
		return false;
	}
	if (!createComputeQueue()) {
		return false;
	}
	
	return true;
}

bool Renderer::createDirectQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_directQueue.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating direct queue");
		exit(-1);
	}
	m_directQueue.Get()->SetName(L"Direct queue");

	for (int i = 0; i < 2; ++i)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_directAllocator[i].GetAddressOf()));
		if (hr != S_OK) {
			printf("Error creating direct allocataor");
			exit(-1);
		}
		std::wstring directAllocatorName = L"Direct Allocator ";
		directAllocatorName.append(std::to_wstring(i));
		m_directAllocator[i].Get()->SetName(directAllocatorName.c_str());

		//Create command list.
		hr = m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_directAllocator[i].Get(),
			nullptr,
			IID_PPV_ARGS(m_graphicsDirectList[i].GetAddressOf())
		);
		if (hr != S_OK) {
			printf("Error creating direct list");
			exit(-1);
		}

		//Command lists are created in the recording state. Since there is nothing to
		//record right now and the main loop expects it to be closed, we close it.
		//hr = m_graphicsDirectList[i].Get()->Close();

		if (hr != S_OK) {
			printf("Error closing direct list at initialisation");
			exit(-1);
		}

		m_graphicsDirectList[i].Get()->SetName(L"Direct queue");
	}

	return true;
}

bool Renderer::createCopyQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_copyQueue.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating copy queue");
		exit(-1);
	}
	m_copyQueue.Get()->SetName(L"Copy queue");

	for (int i = 0; i < 2; ++i)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(m_copyAllocator[i].GetAddressOf()));
		if (hr != S_OK) {
			printf("Error creating copy allocataor");
			exit(-1);
		}
		std::wstring copyAllocatorName = L"Copy Allocator ";
		copyAllocatorName.append(std::to_wstring(i));
		m_copyAllocator[i].Get()->SetName(copyAllocatorName.c_str());
		//Create command list.
		hr = m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_COPY,
			m_copyAllocator[i].Get(),
			nullptr,
			IID_PPV_ARGS(m_graphicsCopyList[i].GetAddressOf())
		);
		if (hr != S_OK) {
			printf("Error creating copy list");
			exit(-1);
		}

		//Command lists are created in the recording state. Since there is nothing to
		//record right now and the main loop expects it to be closed, we close it.
		//hr = m_graphicsCopyList[i].Get()->Close();

		if (hr != S_OK) {
			printf("Error closing copy list at initialisation");
			exit(-1);
		}
		m_graphicsCopyList[i].Get()->SetName(L"Copy queue");
	}

	return true;
}

bool Renderer::createComputeQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_computeQueue.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating compute queue");
		exit(-1);
	}
	m_computeQueue.Get()->SetName(L"Compute queue");
	for (int i = 0; i < 2; ++i)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(m_computeAllocator[i].GetAddressOf()));
		if (hr != S_OK) {
			printf("Error creating compute allocataor");
			exit(-1);
		}		
		m_computeAllocator[i].Get()->SetName(L"Compute allocator");

		//Create command list.
		hr = m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			m_computeAllocator[i].Get(),
			nullptr,
			IID_PPV_ARGS(m_graphicsComputeList[i].GetAddressOf())
		);
		if (hr != S_OK) {
			printf("Error creating compute list");
			exit(-1);
		}

		//Command lists are created in the recording state. Since there is nothing to
		//record right now and the main loop expects it to be closed, we close it.
		//hr = m_graphicsComputeList[i].Get()->Close();

		if (hr != S_OK) {
			printf("Error closing compute list at initialisation");
			exit(-1);
		}
		m_graphicsComputeList[i].Get()->SetName(L"Compute queue");
	}

	return true;
}

bool Renderer::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1* swapChainDesc;

	swapChainDesc = new DXGI_SWAP_CHAIN_DESC1;
	swapChainDesc->Width = m_screenWidth;
	swapChainDesc->Height = m_screenHeight;
	swapChainDesc->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc->Stereo = FALSE;
	swapChainDesc->SampleDesc.Count = 1; //No multisample
	swapChainDesc->SampleDesc.Quality = 0; //Default settings
	swapChainDesc->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc->BufferCount = NUM_SWAP_BUFFERS;
	swapChainDesc->Scaling = DXGI_SCALING_NONE;
	swapChainDesc->SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc->Flags = 0; //Optional
	swapChainDesc->AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	ComPtr<IDXGISwapChain1> swapChain1 = nullptr;

	HRESULT hr = m_factory->CreateSwapChainForHwnd(
		m_directQueue.Get(),
		m_handle,
		swapChainDesc,
		nullptr, //Windowed mode
		nullptr, //restrict to output
		&swapChain1
	);
	if (hr == S_OK)
	{
		if (FAILED(swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain))))
		{
			return false;
		}
	}
	else {
		return false;
	}

	delete swapChainDesc;
	return true;
}

bool Renderer::createFenceAndEventHandle()
{
	//Rendering fence
	HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating fence");
		exit(-1);
	}
	m_fenceValue = 0;
	// Creation of an event handle to use in GPU synchronization
	m_eventHandle = CreateEvent(0, false, false, 0);
	m_fence.Get()->SetName(L"Frame fence");


	//Copy queue fence
	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_copyFence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating copy fence");
		exit(-1);
	}
	// Creation of an event handle to use in GPU synchronization
	m_copyHandle = CreateEvent(0, false, false, 0);
	m_copyThreadHandle = CreateEvent(0, false, false, 0);
	m_copyFence.Get()->SetName(L"Copy recording fence");

	//Compute queue fence
	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_computeFence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating compute fence");
		exit(-1);
	}
	// Creation of an event handle to use in GPU synchronization
	m_computeHandle = CreateEvent(0, false, false, 0);
	m_computeThreadHandle = CreateEvent(0, false, false, 0);
	m_computeFence.Get()->SetName(L"Compute recording fence");

	//Direct queue fence
	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_directFence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating direct fence");
		exit(-1);
	}
	// Creation of an event handle to use in GPU synchronization
	m_directHandle = CreateEvent(0, false, false, 0);
	m_directThreadHandle = CreateEvent(0, false, false, 0);
	m_directFence.Get()->SetName(L"Direct recording fence");

	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_renderingFence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating rendering fence");
		exit(-1);
	}
	// Creation of an event handle to use in GPU synchronization
	m_renderingHandle = CreateEvent(0, false, false, 0);
	m_renderingFence.Get()->SetName(L"Rendering fence");

	return true;
}

bool Renderer::createDescriptorHeap()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	HRESULT hr = m_device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_renderTargetHeap));
	if (hr != S_OK) {
		return false;
	}
	m_renderTargetHeap->SetName(L"Render target Heap");

	//Create descriptor heaps for constant buffers.
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC dhdCB = {};
		//dhdCB.NumDescriptors = NUM_CONSTANT_BUFFERS;		
		dhdCB.NumDescriptors = NUM_BUFFERS_IN_DESC_HEAP;
		dhdCB.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		dhdCB.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		hr = m_device->CreateDescriptorHeap(&dhdCB, IID_PPV_ARGS(&m_cbDescriptorHeaps[i]));
		if (hr != S_OK) {
			return false;
		}
		std::wstring cbName = L"Constant buffer Heap ";
		cbName.append(std::to_wstring(i));
		m_cbDescriptorHeaps[i]->SetName(cbName.c_str());
	}

	//Create descriptor heaps for shader resource views.
	//for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	//{
	//	D3D12_DESCRIPTOR_HEAP_DESC dhdCB = {};
	//	dhdCB.NumDescriptors = NUM_TEXTURE_BUFFERS;		
	//	dhdCB.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//	dhdCB.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//	hr = m_device->CreateDescriptorHeap(&dhdCB, IID_PPV_ARGS(&m_SRVDescriptorHeaps[i]));
	//	if (hr != S_OK) {
	//		return false;
	//	}
	//	std::wstring name = L"Shader resource buffer Heap ";
	//	name.append(std::to_wstring(i));
	//	m_SRVDescriptorHeaps[i]->SetName(name.c_str());
	//}

	//Create descriptor heaps for depth buffer view.
	D3D12_DESCRIPTOR_HEAP_DESC dhdDB = {};
	dhdDB.NumDescriptors = NUM_SWAP_BUFFERS;
	dhdDB.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dhdDB.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hr = m_device->CreateDescriptorHeap(&dhdDB, IID_PPV_ARGS(&m_dbDescriptorHeap));
	if (hr != S_OK) {
		return false;
	}
	m_dbDescriptorHeap->SetName(L"Depth Resource Heap");

	return true;
}

bool Renderer::createRenderTargets()
{
	// Create resources for the render targets.
	m_renderTargetDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart();

	HRESULT hr;
	// One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; ++n)
	{
		hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].GetAddressOf()));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, cdh);
		cdh.ptr += m_renderTargetDescriptorSize;
	}

	if (hr != S_OK)
		return false;

	return true;
}

bool Renderer::createViewportAndScissorRect()
{
	//Viewport
	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;
	m_viewPort.Width = (FLOAT)SCREEN_WIDTH;
	m_viewPort.Height = (FLOAT)SCREEN_HEIGHT;
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;

	//Scissor rectangle
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = SCREEN_WIDTH;
	m_scissorRect.bottom = SCREEN_HEIGHT;

	return true;
}

bool Renderer::createRootSignature()
{
	//Constant Buffer Descriptor Range
	CD3DX12_DESCRIPTOR_RANGE dtRangesCBV[2];
	dtRangesCBV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NUM_CONSTANT_BUFFERS, 0); // b0, b1
	dtRangesCBV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NUM_TEXTURE_BUFFERS, 0);  // t0
	//CD3DX12_DESCRIPTOR_RANGE dtRangesSRV[1];
	//dtRangesSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NUM_TEXTURE_BUFFERS, 0);  // t0



	//Create root parameter
	CD3DX12_ROOT_PARAMETER rootParam[1];
	rootParam[0].InitAsDescriptorTable(ARRAYSIZE(dtRangesCBV), dtRangesCBV, D3D12_SHADER_VISIBILITY_ALL);
	//rootParam[1].InitAsDescriptorTable(ARRAYSIZE(dtRangesSRV), dtRangesSRV, D3D12_SHADER_VISIBILITY_ALL);

	//Create static samplers. (One from GetStaticSamplers in Frank Luna(ch.9, Create). Might want to steal whole function later)
	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,  // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	CD3DX12_ROOT_SIGNATURE_DESC rsDesc = CD3DX12_ROOT_SIGNATURE_DESC(
		ARRAYSIZE(rootParam),	// nr parameters
		rootParam, 				// parameters
		1, 						// nr static samplers
		&linearWrap, 			// static samplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT); // flags


	ID3DBlob* sBlob;
	HRESULT hr = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr
	);
	if (hr != S_OK) {
		return false;
	}

	hr = m_device->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)
	);
	if (hr != S_OK) {
		return false;
	}

	SafeRelease(&sBlob);

	return true;
}
