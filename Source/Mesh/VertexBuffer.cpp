#include "VertexBuffer.h"
#include "..\DXUtility\DXUtility.h"

// Member functions
VertexBuffer::VertexBuffer() // may move content to own function that is called in setData
{
}

VertexBuffer::~VertexBuffer()
{
	SafeRelease(m_vertexBufferResource.GetAddressOf());
	SafeRelease(m_indexBufferResource.GetAddressOf());
	SafeRelease(m_instanceBufferResource.GetAddressOf());
}

void VertexBuffer::setData(const void* data, size_t dataByteSize, const void* indices, size_t indexByteSize, bool instancing)
{
	m_totalSize = dataByteSize; //in bytes
	m_nrOfVertices = m_totalSize / sizeof(Vertex);
	m_totalSizeIndices = indexByteSize;
	m_nrOfIndices = m_totalSizeIndices / sizeof(UINT16);
	m_instancing = instancing;


	bool useOld = false;

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
		m_vertexBufferResource = CreateDefaultBuffer(Renderer::getInstance()->getCopyCommandList(), data, dataByteSize, m_VBUploadHeap, L"VB heap", D3D12_RESOURCE_STATE_COMMON);
		if (m_nrOfIndices > 0)
		{
			m_indexBufferResource = CreateDefaultBuffer(Renderer::getInstance()->getCopyCommandList(), indices, indexByteSize, m_IBUploadHeap, L"IB heap", D3D12_RESOURCE_STATE_COMMON);
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


	// -------------------INSTANCING--------------------------
	if (m_instancing)
	{
		//Buffer
		DirectX::XMFLOAT3 instancePositions[] =
		{
			DirectX::XMFLOAT3(2.0f, -2.0f, -2.0f),
			DirectX::XMFLOAT3(0.0f, -2.0f, -2.0f),
			DirectX::XMFLOAT3(-2.0f, -2.0f, -2.0f),
			DirectX::XMFLOAT3(2.0f, 0.0f, -2.0f),
			DirectX::XMFLOAT3(0.0f, 0.0f, -2.0f),
			DirectX::XMFLOAT3(-2.0f, 0.0f, -2.0f),
			DirectX::XMFLOAT3(2.0f, 2.0f, -2.0f),
			DirectX::XMFLOAT3(0.0f, 2.0f, -2.0f),
			DirectX::XMFLOAT3(-2.0f, 2.0f, -2.0f),
			DirectX::XMFLOAT3(2.0f, -2.0f, 0.0f),
			DirectX::XMFLOAT3(0.0f, -2.0f, 0.0f),
			DirectX::XMFLOAT3(-2.0f, -2.0f, 0.0f),
			DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT3(-2.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT3(2.0f, 2.0f, 0.0f),
			DirectX::XMFLOAT3(0.0f, 2.0f, 0.0f),
			DirectX::XMFLOAT3(-2.0f, 2.0f, 0.0f),
			DirectX::XMFLOAT3(2.0f, -2.0f, 2.0f),
			DirectX::XMFLOAT3(0.0f, -2.0f, 2.0f),
			DirectX::XMFLOAT3(-2.0f, -2.0f, 2.0f),
			DirectX::XMFLOAT3(2.0f, 0.0f, 2.0f),
			DirectX::XMFLOAT3(0.0f, 0.0f, 2.0f),
			DirectX::XMFLOAT3(-2.0f, 0.0f, 2.0f),
			DirectX::XMFLOAT3(2.0f, 2.0f, 2.0f),
			DirectX::XMFLOAT3(0.0f, 2.0f, 2.0f),
			DirectX::XMFLOAT3(-2.0f, 2.0f, 2.0f),
		};
		m_nrOfInstances = ARRAYSIZE(instancePositions);
		m_instanceBufferResource = CreateDefaultBuffer(Renderer::getInstance()->getCopyCommandList(), instancePositions, sizeof(instancePositions), m_instanceUploadHeap, L"Instance heap", D3D12_RESOURCE_STATE_COMMON);

		//View
		m_instanceBufferView.BufferLocation = m_instanceBufferResource->GetGPUVirtualAddress();
		m_instanceBufferView.StrideInBytes = sizeof(DirectX::XMFLOAT3);
		m_instanceBufferView.SizeInBytes = sizeof(instancePositions);
	}
}

void VertexBuffer::bind()
{
	//Location tells us which vertexbuffer we're setting
	Renderer::getInstance()->getDirectCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	
	if (m_nrOfIndices > 0)
		Renderer::getInstance()->getDirectCommandList()->IASetIndexBuffer(&m_indexBufferView);
	if (m_instancing)
		Renderer::getInstance()->getDirectCommandList()->IASetVertexBuffers(1, 1, &m_instanceBufferView);
}

void VertexBuffer::draw()
{
	if (m_nrOfIndices == 0)
		Renderer::getInstance()->getDirectCommandList()->DrawInstanced((UINT)m_nrOfVertices, 1, 0, 0);
	else if (m_instancing)
		Renderer::getInstance()->getDirectCommandList()->DrawIndexedInstanced((UINT)m_nrOfIndices, m_nrOfInstances, 0, 0, 0);
	else
		Renderer::getInstance()->getDirectCommandList()->DrawIndexedInstanced((UINT)m_nrOfIndices, 1, 0, 0, 0);

}

size_t VertexBuffer::getSize()
{
	return m_totalSize;
}
