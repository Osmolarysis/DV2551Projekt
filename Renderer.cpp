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
	return false;
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
