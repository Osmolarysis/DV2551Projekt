#include "DXUtility.h"

ComPtr<ID3D12Resource2> makeBufferHeap(D3D12_HEAP_TYPE type, UINT64 size, LPCWSTR heapName, D3D12_RESOURCE_STATES state)
{
	// upload buffers will want tpo initialize with D3D12_RESOURCE_STATE_GENERIC_READ instead.

	//Do initial stuff
	auto hp = CD3DX12_HEAP_PROPERTIES(type);
	auto rd = CD3DX12_RESOURCE_DESC::Buffer(size);
		
	ComPtr<ID3D12Resource2> resource;
	// create heap
	HRESULT hr = Renderer::getInstance()->getDevice()->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		state,
		nullptr,
		IID_PPV_ARGS(resource.GetAddressOf())
	);
	if (hr != S_OK) {
		printf("Error creating ");
		wprintf(heapName);
		printf("\n");
		
		exit(-1);
	}
	resource->SetName(heapName);
	return resource;
}

ComPtr<ID3D12Resource2> makeTextureHeap(D3D12_HEAP_TYPE type, UINT64 size, LPCWSTR heapName, UINT width, UINT height)
{
	//Do initial stuff
	auto hp = CD3DX12_HEAP_PROPERTIES(type);
	auto rd = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	auto state = (type == D3D12_HEAP_TYPE_DEFAULT) ? D3D12_RESOURCE_STATE_COMMON : D3D12_RESOURCE_STATE_GENERIC_READ;
	ComPtr<ID3D12Resource2> resource;
	// create heap
	HRESULT hr = Renderer::getInstance()->getDevice()->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		state,
		nullptr,
		IID_PPV_ARGS(resource.GetAddressOf())
	);
	if (hr != S_OK) {
		printf("Error creating ");
		wprintf(heapName);
		printf("\n");

		exit(-1);
	}
	resource->SetName(heapName);
	return resource;
}

void setUploadHeapData(ComPtr<ID3D12Resource2> resource, const void* data, size_t size)
{
	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 };	// We do not intend to read from this resource on the CPU.
	HRESULT hr = resource->Map(0, &range, &dataBegin);
	if (hr != S_OK) {
		printf("Error mapping upload heap resources");
		exit(-1);
	}
	memcpy((char*)dataBegin, (char*)data, size);
	resource->Unmap(0, nullptr);
}

void updateBufferHeap(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource2>& uploadBuffer, D3D12_RESOURCE_STATES resourceStateAfter)
{
	// TODO, tror inte behövs med UpdateSubresources
}

// Returns defaultBuffer and uploadBuffer in parameter output.
// uploadBuffer needs to stay in memory until the copy is complete.
ComPtr<ID3D12Resource2> CreateDefaultBuffer(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource2>& uploadBuffer, LPCWSTR name, D3D12_RESOURCE_STATES resourceStateAfter)
{
	// allocates memory
	ComPtr<ID3D12Resource2> defaultHeap = makeBufferHeap(D3D12_HEAP_TYPE_DEFAULT, byteSize, name);
	size_t requiredSize = GetRequiredIntermediateSize(defaultHeap.Get(), 0, 1);
	std::wstring heapName = L"Upload heap ";
	heapName.append(name);
	uploadBuffer = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, requiredSize, heapName.c_str(), D3D12_RESOURCE_STATE_GENERIC_READ);

	// describe data
	D3D12_SUBRESOURCE_DATA vbData = {};
	vbData.pData = initData;
	vbData.RowPitch = byteSize;
	vbData.SlicePitch = vbData.RowPitch;

	UpdateSubresources(cmdList, defaultHeap.Get(), uploadBuffer.Get(), 0, 0, 1, &vbData);

	return defaultHeap;
}

ComPtr<ID3D12Resource2> CreateDefaultTexture(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource2>& uploadBuffer, LPCWSTR name, D3D12_RESOURCE_STATES resourceStateAfter, UINT width, UINT height, UINT pixelSize)
{
	// allocates memory
	ComPtr<ID3D12Resource2> defaultHeap = makeTextureHeap(D3D12_HEAP_TYPE_DEFAULT, byteSize, name, width, height);
	size_t requiredSize = GetRequiredIntermediateSize(defaultHeap.Get(), 0, 1);
	uploadBuffer = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, requiredSize, L"uploadHeap", D3D12_RESOURCE_STATE_GENERIC_READ);

	// describe data
	D3D12_SUBRESOURCE_DATA vbData = {};
	vbData.pData = initData;
	vbData.RowPitch = width * pixelSize;
	vbData.SlicePitch = vbData.RowPitch * height;

	UpdateSubresources(cmdList, defaultHeap.Get(), uploadBuffer.Get(), 0, 0, 1, &vbData);

	return defaultHeap;
}

