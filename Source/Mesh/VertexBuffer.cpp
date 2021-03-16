#include "VertexBuffer.h"

// Helper functions, will probably move. Depending on demand, move to utility class/file
ComPtr<ID3D12Resource2> makeBufferHeap(D3D12_HEAP_TYPE type, UINT64 size, LPCWSTR heapName)	
{
	//Do initial stuff
	auto hp = CD3DX12_HEAP_PROPERTIES(type);
	auto rd = CD3DX12_RESOURCE_DESC::Buffer(size);
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

	// create vertexbuffer	// TODO: change to DEFAULT
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
	//old
	//m_vertexBufferResource = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, m_totalSize, L"vb heap");
	//m_indexBufferResource  = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, m_totalSizeIndices, L"ib heap");

	setUploadHeapData(m_vertexBufferResource, data, m_totalSize);

	if (m_nrOfIndices > 0)
	{
		setUploadHeapData(m_indexBufferResource, indices, m_totalSizeIndices);
	}

	// new

	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.
	//ComPtr<ID3D12Resource2> VBUploadHeap;
	//ComPtr<ID3D12Resource2> IBUploadHeap;
	//CD3DX12_RESOURCE_BARRIER barrier;
	//UINT64 requiredSize = GetRequiredIntermediateSize(m_vertexBufferResource.Get(), 0, 1);

	//VBUploadHeap = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, m_totalSize, L"vb upload heap");
	//VBUploadHeap = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, requiredSize, L"vb upload heap");
	//
	//// Describe data
	//D3D12_SUBRESOURCE_DATA vbData = {};
	//vbData.pData = data;
	//vbData.RowPitch = m_totalSize;
	//vbData.SlicePitch = vbData.RowPitch;

	//// TODO: switch to copy queue/list
	//// transision resource for copying
	//barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBufferResource.Get(),
	//	D3D12_RESOURCE_STATE_COMMON,
	//	D3D12_RESOURCE_STATE_COPY_DEST);
	//Renderer::getInstance()->getGraphicsCommandList()->ResourceBarrier(1, &barrier);

	//// helper funcition to upload data to upload heap and copy to default heap
	//UpdateSubresources(Renderer::getInstance()->getGraphicsCommandList(), m_vertexBufferResource.Get(), VBUploadHeap.Get(), 0, 0, 1, &vbData);

	//// transition resource for using;
	//barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBufferResource.Get(),
	//	D3D12_RESOURCE_STATE_COPY_DEST,
	//	D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	//Renderer::getInstance()->getGraphicsCommandList()->ResourceBarrier(1, &barrier);

	//if (m_nrOfIndices > 0)
	//{
	//	requiredSize = GetRequiredIntermediateSize(m_indexBufferResource.Get(), 0, 1);
	//	IBUploadHeap = makeBufferHeap(D3D12_HEAP_TYPE_UPLOAD, requiredSize, L"ib upload heap");

	//	D3D12_SUBRESOURCE_DATA ibData = {};
	//	ibData.pData = indices;
	//	ibData.RowPitch = m_totalSizeIndices;
	//	ibData.SlicePitch = ibData.RowPitch;

	//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBufferResource.Get(),
	//		D3D12_RESOURCE_STATE_COMMON,
	//		D3D12_RESOURCE_STATE_COPY_DEST);
	//	Renderer::getInstance()->getGraphicsCommandList()->ResourceBarrier(1, &barrier);

	//	UpdateSubresources(Renderer::getInstance()->getGraphicsCommandList(), m_indexBufferResource.Get(), IBUploadHeap.Get(), 0, 0, 1, &ibData);

	//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBufferResource.Get(),
	//		D3D12_RESOURCE_STATE_COPY_DEST,
	//		D3D12_RESOURCE_STATE_INDEX_BUFFER);
	//	Renderer::getInstance()->getGraphicsCommandList()->ResourceBarrier(1, &barrier);
	//}


	// close commandlist and execute list. (maybe move to after all meshes if we have more?)
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
