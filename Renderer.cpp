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
	if (!createSwapChain()) {
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
	if (!createViewportAndScissorRect(SCREEN_WIDTH, SCREEN_HEIGHT)) {
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
	SafeRelease(m_device.GetAddressOf());
	SafeRelease(m_fence.GetAddressOf());
	
	SafeRelease(m_debugController.GetAddressOf());

	CloseHandle(m_eventHandle);
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

bool Renderer::createWindow()
{
	return false;
}

bool Renderer::createDevice()
{
	return false;
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


	return false;
}

bool Renderer::createSwapChain()
{
	return false;
}

bool Renderer::createFenceAndEventHandle()
{
	HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
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
	return false;
}

bool Renderer::createRenderTargets()
{
	return false;
}

bool Renderer::createViewportAndScissorRect(int, int)
{
	return false;
}

bool Renderer::createRootSignature()
{
	return false;
}
