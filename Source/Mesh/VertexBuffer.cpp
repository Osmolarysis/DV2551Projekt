#include "VertexBuffer.h"

// Helper functions, will probably move. Depending on demand, move to utility class/file
ComPtr<ID3D12Resource2> makeBufferHeap(D3D12_HEAP_TYPE type, UINT64 size, LPCWSTR heapName)	
{
	//Do initial stuff
	auto hp = CD3DX12_HEAP_PROPERTIES(type);
	auto rd = CD3DX12_RESOURCE_DESC::Buffer(size);
	auto state = (type == D3D12_HEAP_TYPE_DEFAULT) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ;
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

// Returns default buffer and uploadBuffer in parameter output.
// uploadBuffer needs to stay in memory until the copy is complete.
ComPtr<ID3D12Resource2> CreateDefaultBuffer(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource2>& uploadBuffer, LPCWSTR name, D3D12_RESOURCE_STATES resourceStateAfter)
{
	// allocates memory
	ComPtr<ID3D12Resource2> defaultHeap = makeBufferHeap(D3D12_HEAP_TYPE_DEFAULT, byteSize, name);
	size_t requiredSize = GetRequiredIntermediateSize(defaultHeap.Get(), 0, 1);
	uploadBuffer = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, requiredSize, L"uploadHeap");

	// describe data
	D3D12_SUBRESOURCE_DATA vbData = {};
	vbData.pData = initData;
	vbData.RowPitch = byteSize;
	vbData.SlicePitch = vbData.RowPitch;

	// transision resource is already set as COPY_DEST
	// helper funcition to upload data to upload heap and copy to default heap
	UpdateSubresources(cmdList, defaultHeap.Get(), uploadBuffer.Get(), 0, 0, 1, &vbData);

	// transition resource for using;
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultHeap.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		resourceStateAfter);
	cmdList->ResourceBarrier(1, &barrier);

	return defaultHeap;
}


// Member functions
VertexBuffer::VertexBuffer() // may move content to own function that is called in setData
{
}

VertexBuffer::~VertexBuffer()
{
	SafeRelease(m_vertexBufferResource.GetAddressOf());
	SafeRelease(m_indexBufferResource.GetAddressOf());
}

void VertexBuffer::setData(const void* data, size_t dataByteSize, const void* indices, size_t indexByteSize)
{
	m_totalSize = dataByteSize; //in bytes
	m_nrOfVertices = m_totalSize / sizeof(Vertex);
	m_totalSizeIndices = indexByteSize;
	m_nrOfIndices = m_totalSizeIndices / sizeof(UINT16);


	bool useOld = true;

	// old
	if (useOld)
	{
		m_vertexBufferResource = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, m_totalSize, L"vb heap");
		m_indexBufferResource  = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, m_totalSizeIndices, L"ib heap");

		setUploadHeapData(m_vertexBufferResource, data, m_totalSize);

		if (m_nrOfIndices > 0)
		{
			setUploadHeapData(m_indexBufferResource, indices, m_totalSizeIndices);
		}
	}

	// new

	// allocate and copy data to default heaps
	// TODO: change to copy list
	// TODO: signal copy fence and release upload heap when copy done
	else
	{
		m_vertexBufferResource = CreateDefaultBuffer(Renderer::getInstance()->getDirectCommandList(), data, dataByteSize, m_VBUploadHeap, L"VB heap", D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		if (m_nrOfIndices > 0)
		{
			m_indexBufferResource = CreateDefaultBuffer(Renderer::getInstance()->getDirectCommandList(), indices, indexByteSize, m_IBUploadHeap, L"IB heap", D3D12_RESOURCE_STATE_INDEX_BUFFER);
		}
	}



	//Prepare VB view to be used in bind.
	m_vertexBufferView.BufferLocation = m_vertexBufferResource->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = UINT(sizeof(Vertex));
	m_vertexBufferView.SizeInBytes = UINT(m_totalSize);

	//Prepare IB view to be used in bind if relevant
	if (m_nrOfIndices > 0)
	{
		m_indexBufferView.BufferLocation = m_indexBufferResource->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = UINT(m_totalSizeIndices);
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}


	//// close commandlist and execute list. (maybe move to after all meshes if we have more?)
	//auto hr = Renderer::getInstance()->getGraphicsCommandList()->Close();
	//if (FAILED(hr))
	//{
	//	fprintf(stderr, "Error closing commandlist in vertexbuffer creating");
	//}
	//Renderer::getInstance()->executeList();



	////// wait for GPU to be done. Otherwise need to keep upload heap in memory outside function untill done.
	//Renderer::getInstance()->waitForGPU();

}

void VertexBuffer::bind()
{
	//Location tells us which vertexbuffer we're setting
	Renderer::getInstance()->getDirectCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	if (m_nrOfIndices > 0)
		Renderer::getInstance()->getDirectCommandList()->IASetIndexBuffer(&m_indexBufferView);
}

void VertexBuffer::draw()
{
	if (m_nrOfIndices == 0)
		Renderer::getInstance()->getDirectCommandList()->DrawInstanced((UINT)m_nrOfVertices, 1, 0, 0);
	else
		Renderer::getInstance()->getDirectCommandList()->DrawIndexedInstanced((UINT)m_nrOfIndices, 1, 0, 0, 0);

}

size_t VertexBuffer::getSize()
{
	return m_totalSize;
}
