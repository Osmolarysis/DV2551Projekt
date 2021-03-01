#include "Renderer.h"

Renderer Renderer::m_this(1280, 720);

Renderer::Renderer(int width, int height)
{
	//Set debug mode
	if (!createDebugMode()) {
		printf("error setting debug mode");
		exit(-1);
	}

	//Create window
	if (!createWindow()) {
		printf("error creating window");
		exit(-1);
	}

	//Create device
	if (!createDevice()) {
		printf("error creating device");
		exit(-1);
	}

	//Create command queue and lists
	if (!createCommandQueue()) {
		printf("error creating command queue");
		exit(-1);
	}

	//create swap chain
	if (!createSwapChain()) {
		printf("error creating swap chain");
		exit(-1);
	}

	//create fence and event handler
	if (!createSwapChain()) {
		printf("error creating swapchain and event handler");
		exit(-1);
	}

	//create descriptor heaps
	if (!createDescriptorHeap()) {
		printf("error creating descriptor heap");
		exit(-1);
	}

	//create render target
	if (!createRenderTargets()) {
		printf("error creating render target");
		exit(-1);
	}

	//create view port and scissor rect
	if (!createViewportAndScissorRect(SCREEN_WIDTH, SCREEN_HEIGHT)) {
		printf("error creating view port and scissor rect");
		exit(-1);
	}

	//create root signature
	if (!createRootSignature()) {
		printf("error creating root signature");
		exit(-1);
	}
}

Renderer::~Renderer()
{
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
	return false;
}

bool Renderer::createCommandQueue()
{
	//One of each type of queue (3 total), but six lists (possible to go 3 lists with 6 allocators, but hard appearently)
	//Start with grade D, 1 Direct queue and 2 lists
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

	return 1;
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
