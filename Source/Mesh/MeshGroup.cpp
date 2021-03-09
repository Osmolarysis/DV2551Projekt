#include "MeshGroup.h"

MeshGroup::MeshGroup(LPCWSTR shaderFiles[], UINT cbufferSize, UINT cbufferLocation)
{
	std::string errorStr;
	m_vsBlob = compileShader(shaderFiles[0], errorStr, ShaderType::VS);
	m_psBlob = compileShader(shaderFiles[1], errorStr, ShaderType::PS);
	m_cbuffer = std::make_unique<ConstantBuffer>(ConstantBuffer(cbufferSize, cbufferLocation));
	makePipelineStateObject();
}

MeshGroup::~MeshGroup()
{
	SafeRelease(m_pipelineStateObject.GetAddressOf());
}

void MeshGroup::addMesh(std::shared_ptr<Mesh> mesh)
{
	m_meshes.push_back(mesh);
}

void addMesh(std::shared_ptr<VertexBuffer> vb)
{
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
	mesh->addIAVertexBufferBinding(vb);
}


void MeshGroup::drawAll()
{
	// bind pipeline state object
	Renderer::getInstance()->getGraphicsCommandList()->SetPipelineState(m_pipelineStateObject.Get());

	// Theoreticly constantfuffer seems to bind automaticlly? (a least with 1 obj, probably)

	// draw all meshes
	for (auto& mesh : m_meshes)
	{
		m_cbuffer->setData((void*)mesh->getTransform());
		mesh->draw();
	}
}

ComPtr<ID3DBlob> MeshGroup::compileShader(LPCWSTR shaderFile, std::string& errString, ShaderType type)
{
	std::string shaderModel;
	std::string entryPoint;
	switch (type)
	{
	case ShaderType::VS:
		shaderModel = "vs_5_0";
		entryPoint = "VS_main";
		break;
	case ShaderType::PS:
		shaderModel = "ps_5_0";
		entryPoint = "PS_main";
		break;
	default:
		break;
	}

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors = nullptr;

	HRESULT hr;
	hr = D3DCompileFromFile(shaderFile,
		nullptr,		// Optional macros. Here we can use the #ifdef trick from the assignment 
		nullptr,		// Optional include
		entryPoint.c_str(),
		shaderModel.c_str(),
		0,				// shader compile options
		0,				// effect compile options
		&byteCode,
		&errors);

	if (errors != nullptr)
	{
		OutputDebugStringA((char*)errors->GetBufferPointer());
	}

	return byteCode;
}

bool MeshGroup::makePipelineStateObject()
{
	// Make renderstate for real this time
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	// TODO: add normal and uv

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	// Pipeline state //
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	// Specify pipeline stages:
	gpsd.pRootSignature = Renderer::getInstance()->getRootSignature();
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(m_vsBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = m_vsBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(m_psBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = m_psBlob->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	//if (r->getwireFrame()) gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	HRESULT hr = Renderer::getInstance()->getDevice()->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&m_pipelineStateObject));
	if (hr != S_OK) {
		printf("Error creating graphics pipeline state\n");
		exit(-1);
	}
	return true;
}

