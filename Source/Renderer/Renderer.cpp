﻿#include "Renderer.h"
#include "..\Utility\Timer.h"
#include "..\Utility\Input.h"
#include "..\DXUtility\DXUtility.h"


Renderer Renderer::m_this(1280, 720);

Renderer::Renderer(int width, int height)
{
	m_screenWidth = width;
	m_screenHeight = height;

	//Set debug mode
	/*if (!createDebugMode()) {
		printf("error setting debug mode\n");
		exit(-1);
	}*/

	//Create window
	if (!createWindow()) {
		printf("error creating window\n");
		exit(-1);
	}

	//Create device
	HRESULT hr = D3D12EnableExperimentalFeatures(0, nullptr, nullptr, nullptr);
	if (hr != S_OK) {
		std::cout << "failed enabling experimental features" << std::endl;
		exit(-1);
	}
	if (!createDevice()) {
		printf("error creating device\n");
		exit(-1);
	}
	if (isDeveloperModeOn()) {
		hr = m_device->SetStablePowerState(true);
		if (hr != S_OK) {
			std::cout << "failed setting stable power state" << std::endl;
			exit(-1);
		}
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
	//Create query heaps
	if (!createQueryHeaps()) {
		printf("Error creating query heaps\n");
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
	for (size_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		SafeRelease(m_fence[i].GetAddressOf());
	}
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

		std::wstring name = L"Depth stencil buffer ";
		name.append(std::to_wstring(i));
		m_depthStencilBuffer[i].Get()->SetName(name.c_str());

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

ID3D12CommandAllocator* Renderer::getDirectCommandAllocator(int index)
{
	return m_directAllocator[index].Get();
}

ID3D12QueryHeap* Renderer::getDirectQueryHeap(int index)
{
	return m_directQueryHeap[index].Get();
}

ID3D12Resource1* Renderer::getDirectQueryResult(int index)
{
	return m_directQueryResult[index].Get();
}

D3D12_VIEWPORT* Renderer::getViewPort()
{
	return &m_viewPort;
}

D3D12_RECT* Renderer::getScissorRect()
{
	return &m_scissorRect;
}

ID3D12Resource1* Renderer::getRenderTarget(int index)
{
	return m_renderTargets[index].Get();
}

ID3D12DescriptorHeap* Renderer::getRenderTargetHeap()
{
	return m_renderTargetHeap.Get();
}

size_t Renderer::getRenderTargetHeapSize()
{
	return m_renderTargetDescriptorSize;
}

ID3D12DescriptorHeap* Renderer::getDepthBufferHeap()
{
	return m_dbDescriptorHeap.Get();
}

size_t Renderer::getDepthBufferHeapSize()
{
	return m_depthBufferDescriptorSize;
}

ID3D12DescriptorHeap* Renderer::getConstantBufferHeap(int index)
{
	return m_cbDescriptorHeaps[index].Get();
}

bool Renderer::getSingleThreaded()
{
	return m_singleThread;
}

void Renderer::closeCommandLists()
{
	HRESULT hr;

	for (size_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		SetResourceTransitionBarrier(
			m_graphicsDirectList[i].Get(),
			m_depthStencilBuffer[i].Get(),
			D3D12_RESOURCE_STATE_COMMON,		//state before
			D3D12_RESOURCE_STATE_DEPTH_WRITE	//state after
		);
	}

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

	for (size_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		ID3D12CommandList* copyListsToExecute[] = { m_graphicsCopyList[i].Get() };
		m_copyQueue->ExecuteCommandLists(ARRAYSIZE(copyListsToExecute), copyListsToExecute);
		m_copyQueue->Signal(m_fence[i].Get(), 1);

		m_computeQueue->Wait(m_fence[i].Get(), 1);
		ID3D12CommandList* computeListsToExecute[] = { m_graphicsComputeList[i].Get() };
		m_computeQueue->ExecuteCommandLists(ARRAYSIZE(computeListsToExecute), computeListsToExecute);
		m_computeQueue->Signal(m_fence[i].Get(), 2);

		m_directQueue->Wait(m_fence[i].Get(), 2);
		ID3D12CommandList* directListsToExecute[] = { m_graphicsDirectList[i].Get() };
		m_directQueue->ExecuteCommandLists(ARRAYSIZE(directListsToExecute), directListsToExecute);

		m_fenceValue[i] = 3;
		m_frameComplete[i] = 3;
		m_directQueue->Signal(m_fence[i].Get(), m_fenceValue[i]);
	}
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

	//Toggle wait for GPU and single threaded recording
	if (Input::getInstance()->keyPressed(DirectX::Keyboard::Keys::G)) {
		m_waitForGPU = !m_waitForGPU;
		if (m_waitForGPU)
			std::cout << "Single buffering" << std::endl;
		else if (!m_waitForGPU)
			std::cout << "Double buffering" << std::endl;
	}
	if (Input::getInstance()->keyPressed(DirectX::Keyboard::Keys::T)) {
		m_singleThread = !m_singleThread;
		if (m_singleThread)
			std::cout << "Recording command lists on one thread" << std::endl;
		else if (!m_singleThread)
			std::cout << "Recording command lists on multiple threads" << std::endl;
	}
	if (m_waitForGPU)
		backBufferIndex = !backBufferIndex;

	UINT64 lastFinishedQueue = m_fence[backBufferIndex].Get()->GetCompletedValue(); //Number of last finished queue

	//Wait
	std::chrono::steady_clock::time_point beforeWait = Timer::getInstance()->timestamp();
	if (m_frameComplete[backBufferIndex] > lastFinishedQueue) {
		HRESULT hr = m_fence[backBufferIndex].Get()->SetEventOnCompletion(m_frameComplete[backBufferIndex], m_eventHandle[backBufferIndex]);
		WaitForSingleObject(m_eventHandle[backBufferIndex], INFINITE);
	}
	std::chrono::steady_clock::time_point afterWait = Timer::getInstance()->timestamp();
	Timer::getInstance()->logCPUtime(Timer::WAITFORPREVIOUSFRAME, beforeWait, afterWait);

	//Recording data
	UINT64 queueTimes[6];
	getQueueTimes(queueTimes);
	Timer::getInstance()->logExactGPUtimes(queueTimes);
	Timer::getInstance()->logGPUtime(queueTimes[1] - queueTimes[0], queueTimes[3] - queueTimes[2], queueTimes[5] - queueTimes[4], queueTimes[5] - queueTimes[0]);
}

void Renderer::executeList()
{
	Timer* timer = Timer::getInstance();
	UINT backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	//Wait for Copy queue to finish recording
	std::chrono::steady_clock::time_point beforeWaitCopy = timer->timestamp();
	if (!m_singleThread)
		WaitForSingleObject(m_copyHandle, INFINITE);
	std::chrono::steady_clock::time_point afterWaitCopy = timer->timestamp();
	timer->logCPUtime(Timer::WAITFORCOPYRECORD, beforeWaitCopy, afterWaitCopy);

	//Execute Copy queue
	ID3D12CommandList* listsToExecuteCopy[] = { m_graphicsCopyList[backBufferIndex].Get() };
	m_copyQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecuteCopy), listsToExecuteCopy);

	m_fenceValue[backBufferIndex]++;
	UINT64 copyQueueFinished = m_fenceValue[backBufferIndex];

	m_copyQueue->Signal(m_fence[backBufferIndex].Get(), copyQueueFinished);

	//Wait for Compute queue to finish recording
	std::chrono::steady_clock::time_point beforeWaitCompute = timer->timestamp();
	if (!m_singleThread)
		WaitForSingleObject(m_computeHandle, INFINITE);
	std::chrono::steady_clock::time_point afterWaitCompute = timer->timestamp();
	timer->logCPUtime(Timer::WAITFORCOMPUTERECORD, beforeWaitCompute, afterWaitCompute);

	//Wait for the previous compute to finish and the current copy
	m_computeQueue->Wait(m_fence[backBufferIndex].Get(), copyQueueFinished); // Wait for copy to finished to start compute

	//Execute Compute queue
	ID3D12CommandList* listsToExecuteCompute[] = { m_graphicsComputeList[backBufferIndex].Get() };
	m_computeQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecuteCompute), listsToExecuteCompute);

	m_fenceValue[backBufferIndex]++;
	UINT64 computeQueueFinished = m_fenceValue[backBufferIndex];
	m_computeQueue->Signal(m_fence[backBufferIndex].Get(), computeQueueFinished); //Done

	//Wait for Direct queue to finish recording
	std::chrono::steady_clock::time_point beforeWaitDirect = timer->timestamp();
	if (!m_singleThread)
		WaitForSingleObject(m_directHandle, INFINITE);
	std::chrono::steady_clock::time_point afterWaitDirect = timer->timestamp();
	timer->logCPUtime(Timer::WAITFORDIRECTRECORD, beforeWaitDirect, afterWaitDirect);

	//Execute the commands!
	ID3D12CommandList* listsToExecute[] = { m_graphicsDirectList[backBufferIndex].Get() };
	m_directQueue->Wait(m_fence[backBufferIndex].Get(), computeQueueFinished);
	m_directQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	m_fenceValue[backBufferIndex]++;
	m_frameComplete[backBufferIndex] = m_fenceValue[backBufferIndex]; //Direct finished

	//Set finished rendering value
	m_directQueue.Get()->Signal(m_fence[backBufferIndex].Get(), m_frameComplete[backBufferIndex]);
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
	for (size_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		//Wait for copy queue
		const UINT64 fenceCopy = m_fenceValue[i];
		m_copyQueue->Signal(m_fence[i].Get(), fenceCopy);
		m_fenceValue[i]++;

		//Wait until copy queue is done.
		if (m_fence[i]->GetCompletedValue() < fenceCopy)
		{
			m_fence[i]->SetEventOnCompletion(fenceCopy, m_eventHandle);
			WaitForSingleObject(m_eventHandle, INFINITE);
		}

		//Wait for compute queue
		const UINT64 fenceCompute = m_fenceValue[i];
		m_computeQueue->Signal(m_fence[i].Get(), fenceCompute);
		m_fenceValue[i]++;

		//Wait until compute queue is done.
		if (m_fence[i]->GetCompletedValue() < fenceCompute)
		{
			m_fence[i]->SetEventOnCompletion(fenceCompute, m_eventHandle);
			WaitForSingleObject(m_eventHandle, INFINITE);
		}

		const UINT64 fenceDirect = m_fenceValue[i];
		m_directQueue->Signal(m_fence[i].Get(), fenceDirect);
		m_fenceValue[i]++;

		//Wait until direct queue is done.
		if (m_fence[i]->GetCompletedValue() < fenceDirect)
		{
			m_fence[i]->SetEventOnCompletion(fenceDirect, m_eventHandle);
			WaitForSingleObject(m_eventHandle, INFINITE);
		}
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

bool Renderer::isDeveloperModeOn()
{
	HKEY hKey;
	auto err = RegOpenKeyExW(HKEY_LOCAL_MACHINE, LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\AppModelUnlock)", 0, KEY_READ, &hKey);
	if (err != ERROR_SUCCESS)
		return false;
	DWORD value{};
	DWORD dwordSize = sizeof(DWORD);
	err = RegQueryValueExW(hKey, L"AllowDevelopmentWithoutDevLicense", 0, NULL, reinterpret_cast<LPBYTE>(&value), &dwordSize);
	RegCloseKey(hKey);
	if (err != ERROR_SUCCESS)
		return false;
	return value != 0;
}

void Renderer::getQueueTimes(UINT64* dest)
{
	UINT backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	void* sourceData;
	D3D12_RANGE readRange = { 0, 6 * sizeof(UINT64) };
	D3D12_RANGE writeRange = { 0, 0 };

	m_directQueryResult[backBufferIndex].Get()->Map(0, &readRange, &sourceData);
	memcpy(dest, sourceData, 6 * sizeof(UINT64));

	m_directQueryResult[backBufferIndex].Get()->Unmap(0, &writeRange);
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

ID3D12GraphicsCommandList* Renderer::getCopyCommandList(int index)
{
	return m_graphicsCopyList[index].Get();
}

ID3D12CommandAllocator* Renderer::getCopyCommandAllocator(int index)
{
	return m_copyAllocator[index].Get();
}

ID3D12QueryHeap* Renderer::getCopyQueryHeap(int index)
{
	return m_copyQueryHeap[index].Get();
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

ID3D12GraphicsCommandList* Renderer::getComputeCommandList(int index)
{
	return m_graphicsComputeList[index].Get();
}

ID3D12CommandAllocator* Renderer::getComputeCommandAllocator(int index)
{
	return m_computeAllocator[index].Get();
}

ID3D12QueryHeap* Renderer::getComputeQueryHeap(int index)
{
	return m_computeQueryHeap[index].Get();
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
		m_device.Get()->SetName(L"Device");
	}
	else {
		// No adapter with level 12.1
		return false;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS3 featureDataOptions;
	m_device.Get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &featureDataOptions, sizeof(featureDataOptions)); //This is making me sad.

	return true;
}

bool Renderer::createDebugMode()
{
	HMODULE mD3D12 = LoadLibrary(L"D3D12.dll");
	PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
	if (SUCCEEDED(f(IID_PPV_ARGS(m_debugController.GetAddressOf())))) {
		m_debugController->EnableDebugLayer();
		m_debugController->SetEnableGPUBasedValidation(true);
		//m_debugController->SetEnableAutoDebugName(true);
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
		std::wstring name = L"Direct list ";
		name.append(std::to_wstring(i));
		hr = m_graphicsDirectList[i].Get()->SetName(name.c_str());
		if (hr != S_OK) {
			printf("Error naming direct list %u", i);
			exit(-1);
		}
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
		std::wstring name = L"Copy list ";
		name.append(std::to_wstring(i));
		hr = m_graphicsCopyList[i].Get()->SetName(name.c_str());
		if (hr != S_OK) {
			printf("Error naming copy list %u", i);
			exit(-1);
		}
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
		std::wstring name = L"Compute list ";
		name.append(std::to_wstring(i));
		hr = m_graphicsComputeList[i].Get()->SetName(name.c_str());
		if (hr != S_OK) {
			printf("Error naming compute list %u", i);
			exit(-1);
		}
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
	HRESULT hr;
	//Rendering fence
	for (size_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence[i].GetAddressOf()));
		if (hr != S_OK) {
			printf("Error creating fence");
			exit(-1);
		}
		m_fenceValue[i] = 0;
		// Creation of an event handle to use in GPU synchronization
		m_eventHandle[i] = CreateEvent(0, false, false, 0);
		std::wstring name = L"Frame fence ";
		name.append(std::to_wstring(i));
		hr = m_fence[i].Get()->SetName(name.c_str());
		if (hr != S_OK) {
			printf("Error naming frame fence %i", (int)i);
			exit(-1);
		}
	}


	//Copy queue fence
	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_copyFence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating copy fence");
		exit(-1);
	}
	// Creation of an event handle to use in GPU synchronization
	m_copyHandle = CreateEvent(0, false, false, 0);
	m_copyThreadHandle = CreateEvent(0, false, false, 0);
	hr = m_copyFence.Get()->SetName(L"Copy recording fence");
	if (hr != S_OK) {
		printf("Error naming copy fence");
		exit(-1);
	}

	//Compute queue fence
	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_computeFence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating compute fence");
		exit(-1);
	}
	// Creation of an event handle to use in GPU synchronization
	m_computeHandle = CreateEvent(0, false, false, 0);
	m_computeThreadHandle = CreateEvent(0, false, false, 0);
	hr = m_computeFence.Get()->SetName(L"Compute recording fence");
	if (hr != S_OK) {
		printf("Error naming compute fence");
		exit(-1);
	}

	//Direct queue fence
	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_directFence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating direct fence");
		exit(-1);
	}
	// Creation of an event handle to use in GPU synchronization
	m_directHandle = CreateEvent(0, false, false, 0);
	m_directThreadHandle = CreateEvent(0, false, false, 0);
	hr = m_directFence.Get()->SetName(L"Direct recording fence");
	if (hr != S_OK) {
		printf("Error naming direct fence");
		exit(-1);
	}

	////Game logic fence
	//hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_gameLogicFence.GetAddressOf()));
	//if (hr != S_OK) {
	//	printf("Error creating game logic fence");
	//	exit(-1);
	//}
	//// Creation of an event handle to use in GPU synchronization
	//hr = m_gameLogicFence.Get()->SetName(L"Game logic fence");
	//if (hr != S_OK) {
	//	printf("Error naming game logic fence");
	//	exit(-1);
	//}
	//m_lastFinishedGameLogicUpdate = 0;

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
		std::wstring name = L"Render target ";
		name.append(std::to_wstring(n));
		m_renderTargets[n].Get()->SetName(name.c_str());
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
	CD3DX12_ROOT_PARAMETER rootParam[3];
	rootParam[0].InitAsDescriptorTable(ARRAYSIZE(dtRangesCBV), dtRangesCBV, D3D12_SHADER_VISIBILITY_ALL);
	//rootParam[1].InitAsDescriptorTable(ARRAYSIZE(dtRangesSRV), dtRangesSRV, D3D12_SHADER_VISIBILITY_ALL);
	rootParam[1].InitAsUnorderedAccessView(0);
	rootParam[2].InitAsUnorderedAccessView(1);

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
	m_rootSignature.Get()->SetName(L"Root signature");

	SafeRelease(&sBlob);

	return true;
}

bool Renderer::createQueryHeaps()
{
	HRESULT hr;
	for (size_t i = 0; i < NUM_COMMANDLISTS; i++)
	{
		D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
		queryHeapDesc.Count = 4;
		queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		hr = m_device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(m_directQueryHeap[i].GetAddressOf()));
		if (hr != S_OK)
		{
			return false;
		}
		hr = m_device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(m_computeQueryHeap[i].GetAddressOf()));
		if (hr != S_OK)
		{
			return false;
		}
		D3D12_QUERY_HEAP_DESC queryHeapDesc_copy = {};
		queryHeapDesc_copy.Count = 2;
		queryHeapDesc_copy.Type = D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP;
		hr = m_device->CreateQueryHeap(&queryHeapDesc_copy, IID_PPV_ARGS(m_copyQueryHeap[i].GetAddressOf()));
		if (hr != S_OK)
		{
			return false;
		}

		std::wstring name = L"Direct query result ";
		name.append(std::to_wstring(i));
		m_directQueryResult[i] = makeBufferHeap(D3D12_HEAP_TYPE_READBACK, 6 * sizeof(UINT64), name.c_str(), D3D12_RESOURCE_STATE_COPY_DEST);
	}
	return true;
}