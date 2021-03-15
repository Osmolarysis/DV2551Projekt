#include "VertexBuffer.h"

// Helper functions, will probably move. Depending on demand, move to utility class/file
ComPtr<ID3D12Resource2> makeBufferHeap(D3D12_HEAP_TYPE type, UINT64 size, LPCWSTR heapName)	
{
	//Do initial stuff
	auto hp = CD3DX12_HEAP_PROPERTIES(type);
	auto rd = CD3DX12_RESOURCE_DESC::Buffer(size);
	ComPtr<ID3D12Resource2> resource;
	// create heap
	HRESULT hr = Renderer::getInstance()->getDevice()->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(resource.GetAddressOf())
	);
	if (hr != S_OK) {
		printf("Error creating vertex buffer\n");
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
		printf("Error mapping vertex buffer resources");
		exit(-1);
	}
	memcpy((char*)dataBegin, (char*)data, size);
	resource->Unmap(0, nullptr);
}


// Member functions
VertexBuffer::VertexBuffer(size_t size, size_t sizeIndexData) // may move content to own function that is called in setData
{
	m_totalSize = size; //in bytes
	m_nrOfVertices = size / sizeof(Vertex);
	m_nrOfIndices = sizeIndexData / sizeof(UINT16);
	m_totalSizeIndices = sizeIndexData;

	// create vertexbuffer
	m_vertexBufferResource = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, size, L"vb heap");
	//Prepare VB view to be used in bind.
	m_vertexBufferView.BufferLocation = m_vertexBufferResource->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = UINT(sizeof(Vertex));
	m_vertexBufferView.SizeInBytes = UINT(m_totalSize);

	// create index buffer
	if (m_nrOfIndices > 0)
	{
		m_indexBufferResource = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, sizeIndexData, L"ib heap");
		//Prepare IB view to be used in bind.
		m_indexBufferView.BufferLocation = m_indexBufferResource->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = UINT(m_totalSizeIndices);
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}
}

VertexBuffer::~VertexBuffer()
{
	SafeRelease(m_vertexBufferResource.GetAddressOf());
	SafeRelease(m_indexBufferResource.GetAddressOf());
}

void VertexBuffer::setData(const void* data, const void* indices)
{
	setUploadHeapData(m_vertexBufferResource, data, m_totalSize);

	if (m_nrOfIndices > 0 && indices != nullptr)
	{
		setUploadHeapData(m_indexBufferResource, indices, m_totalSizeIndices);
	}
}

void VertexBuffer::bind()
{
	//Location tells us which vertexbuffer we're setting
	Renderer::getInstance()->getGraphicsCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	if (m_nrOfIndices > 0)
		Renderer::getInstance()->getGraphicsCommandList()->IASetIndexBuffer(&m_indexBufferView);
}

void VertexBuffer::draw()
{
	if (m_nrOfIndices == 0)
		Renderer::getInstance()->getGraphicsCommandList()->DrawInstanced((UINT)m_nrOfVertices, 1, 0, 0);
	else
		Renderer::getInstance()->getGraphicsCommandList()->DrawIndexedInstanced((UINT)m_nrOfIndices, 1, 0, 0, 0);

}

size_t VertexBuffer::getSize()
{
	return m_totalSize;
}
