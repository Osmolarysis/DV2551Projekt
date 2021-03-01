#include "Renderer.h"

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
		printf("error creating swapchain and event handler\n");
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
Renderer* Renderer::getInstance()
{
	return &m_this;
}

unsigned int Renderer::getScreenWidth() const
{
	return m_screenWidth;
}

unsigned int Renderer::getScreenHeight() const
{
	return m_screenHeight;
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
	wc.lpszClassName = m_windowTitle;
	RegisterClass(&wc);

	// Create the window
	m_handle = CreateWindow(m_windowTitle, m_windowTitle,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, 0, m_screenWidth,
		m_screenHeight + 30, nullptr, nullptr, nullptr, nullptr);

	return true;
}

bool Renderer::createDevice() // DXR support is assumed... todo
{
	ComPtr<IDXGIAdapter1> adapter; //Want to go four R£££

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

		hr = m_graphicsCommandList[i]->Close();

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
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain)))) {
			swapChain1->Release();
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
	m_fenceValue = 1;
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
	//Define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE dtRangesCBV[1];
	dtRangesCBV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRangesCBV[0].NumDescriptors = 1;			// WVP matrix
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
