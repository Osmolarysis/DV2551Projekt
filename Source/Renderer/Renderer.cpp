#include "Renderer.h"
#include "..\Utility\Timer.h"


Renderer Renderer::m_this(1280, 720);

Renderer::Renderer(int width, int height)
{
	m_screenWidth = width;
	m_screenHeight = height;

	for (int i = 0; i < NUM_CONSTANT_BUFFERS; i++) {
		m_cbDescriptorSize[i] = 0;
	}

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
	if (!createCommandQueue()) {
		printf("error creating command queue\n");
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

	//Commandqueue/list/allocator
	SafeRelease(m_commandQueue.GetAddressOf());
	for (size_t i = 0; i < NUM_COMMANDLISTS; i++)
	{
		SafeRelease(m_commandAllocator[i].GetAddressOf());
		SafeRelease(m_graphicsCommandList[i].GetAddressOf());
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

ID3D12GraphicsCommandList* Renderer::getGraphicsCommandList()
{
	int index = m_swapChain->GetCurrentBackBufferIndex();
	return m_graphicsCommandList[index].Get();
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

	HRESULT hr = m_commandAllocator[backBufferIndex].Get()->Reset();
	if (hr != S_OK) {
		printf("Error reseting command allocator\n");
		exit(-1);
	}

	hr = m_graphicsCommandList[backBufferIndex].Get()->Reset(m_commandAllocator[backBufferIndex].Get(), nullptr);
	if (hr != S_OK) {
		printf("Error reseting command list\n");
		exit(-1);
	}

	//Set necessary states.
	m_graphicsCommandList[backBufferIndex].Get()->RSSetViewports(1, &m_viewPort);
	m_graphicsCommandList[backBufferIndex].Get()->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(
		m_graphicsCommandList[backBufferIndex].Get(),
		m_renderTargets[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);

	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetHeap.Get()->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += (SIZE_T)m_renderTargetDescriptorSize * (SIZE_T)backBufferIndex;

	m_graphicsCommandList[backBufferIndex].Get()->ClearRenderTargetView(cdh,
		clearColour, 0, nullptr);

	if (!dsvSetWrite[backBufferIndex]) { //TODO: Find a better way than this, maybe "gpu warm up" function
		SetResourceTransitionBarrier(
			m_graphicsCommandList[backBufferIndex].Get(),
			m_depthStencilBuffer[backBufferIndex].Get(),
			D3D12_RESOURCE_STATE_COMMON,		//state before
			D3D12_RESOURCE_STATE_DEPTH_WRITE	//state after
		);
		dsvSetWrite[backBufferIndex] = true;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DBcdh = m_dbDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart();
	DBcdh.ptr += (SIZE_T)m_depthBufferDescriptorSize * (SIZE_T)backBufferIndex;

	m_graphicsCommandList[backBufferIndex].Get()->ClearDepthStencilView(DBcdh, D3D12_CLEAR_FLAG_DEPTH /*| D3D12_CLEAR_FLAG_STENCIL*/, 1.0f, 0, 1, &m_scissorRect);

	// Specify the buffers we are going to render to. Correct render target?
	m_graphicsCommandList[backBufferIndex].Get()->OMSetRenderTargets(1, &cdh, true, &DBcdh);

	//Set root signature
	m_graphicsCommandList[backBufferIndex].Get()->SetGraphicsRootSignature(m_rootSignature.Get());

	m_graphicsCommandList[backBufferIndex].Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Set constant buffer descriptor heaps
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbDescriptorHeaps[backBufferIndex].Get() };
	m_graphicsCommandList[backBufferIndex].Get()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);
	m_graphicsCommandList[backBufferIndex].Get()->SetGraphicsRootDescriptorTable(0, m_cbDescriptorHeaps[backBufferIndex].Get()->GetGPUDescriptorHandleForHeapStart());
}

void Renderer::executeList()
{
	UINT backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	SetResourceTransitionBarrier(m_graphicsCommandList[backBufferIndex].Get(),
		m_renderTargets[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	HRESULT hr = m_graphicsCommandList[backBufferIndex].Get()->Close();
	if (hr != S_OK) {
		printf("Error closing command list %i\n", backBufferIndex);
		exit(-1);
	}

	//Execute the commands!
	ID3D12CommandList* listsToExecute[] = { m_graphicsCommandList[backBufferIndex].Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	m_fenceValue++;

	//Set finished rendering value
	m_commandQueue.Get()->Signal(m_fence.Get(), m_fenceValue);
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
	double fps = 1 / dt;
	std::wstring fps_str = std::to_wstring(fps);
	m_windowTitle = newTitle + L" " + fps_str;

	SetWindowText(m_handle, m_windowTitle.c_str());
}

void Renderer::waitForGPU()
{
	const UINT64 fence = m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), fence);
	m_fenceValue++;

	//Wait until command queue is done.
	if (m_fence->GetCompletedValue() < fence)
	{
		m_fence->SetEventOnCompletion(fence, m_eventHandle);
		WaitForSingleObject(m_eventHandle, INFINITE);
	}
}

ID3D12DescriptorHeap* Renderer::getCBDescriptorHeap(UINT bufferIndex)
{
	return m_cbDescriptorHeaps[bufferIndex].Get();
}

UINT Renderer::getCBDescriptorSize(UINT bufferIndex) const
{
	return m_cbDescriptorSize[bufferIndex];
}

void Renderer::setCBDescriptorSize(UINT location, UINT size)
{
	m_cbDescriptorSize[location] = size;
}

void Renderer::setFence(int fence, int value)
{
	m_commandQueue.Get()->Signal(m_fence.Get(), value);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
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

bool Renderer::createCommandQueue()
{
	//One of each type of queue (3 total), but six lists (possible to go 3 lists with 6 allocators, but hard appearently)
	//Start with grade D, 1 Direct queue and 2 lists

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating command queue");
		exit(-1);
	}

	for (int i = 0; i < 2; ++i)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator[i].GetAddressOf()));
		if (hr != S_OK) {
			printf("Error creating command allocataor");
			exit(-1);
		}

		//Create command list.
		hr = m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_commandAllocator[i].Get(),
			nullptr,
			IID_PPV_ARGS(m_graphicsCommandList[i].GetAddressOf())
		);
		if (hr != S_OK) {
			printf("Error creating command list");
			exit(-1);
		}


		//Command lists are created in the recording state. Since there is nothing to
		//record right now and the main loop expects it to be closed, we close it.

		hr = m_graphicsCommandList[i].Get()->Close();

		if (hr != S_OK) {
			printf("Error closing command list at initialisation");
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
		m_commandQueue.Get(),
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
	HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
	if (hr != S_OK) {
		printf("Error creating fence");
		exit(-1);
	}
	m_fenceValue = 0;
	// Creation of an event handle to use in GPU synchronization
	m_eventHandle = CreateEvent(0, false, false, 0);

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
		dhdCB.NumDescriptors = NUM_CONSTANT_BUFFERS;
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
	D3D12_DESCRIPTOR_RANGE dtRangesCBV[1];
	dtRangesCBV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRangesCBV[0].NumDescriptors = NUM_CONSTANT_BUFFERS;
	dtRangesCBV[0].BaseShaderRegister = 0;		// Base shader register b0
	dtRangesCBV[0].RegisterSpace = 0;
	dtRangesCBV[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//Create descriptor table(s)
	D3D12_ROOT_DESCRIPTOR_TABLE rdtCBV;
	rdtCBV.NumDescriptorRanges = ARRAYSIZE(dtRangesCBV);
	rdtCBV.pDescriptorRanges = dtRangesCBV;

	//Create root parameter
	D3D12_ROOT_PARAMETER rootParam[1];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = rdtCBV;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;

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
