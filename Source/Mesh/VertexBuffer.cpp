#include "VertexBuffer.h"
#include "..\DXUtility\DXUtility.h"
#include "..\Utility\Input.h"

DirectX::XMFLOAT3* VertexBuffer::createInstances(int width, int height, int depth)
{
	m_nrOfInstances = width * height * depth;
	DirectX::XMFLOAT3* instancePositions = new DirectX::XMFLOAT3[m_nrOfInstances];
	srand(unsigned int(time(NULL)));
	int index = 0;
	float averageDistanceBetweenCubes = 4.0f;
	float posX, posY, posZ = 0.0f;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				index = x + width * (y + depth * z);
				posX = x * averageDistanceBetweenCubes - width + 1 + (rand() % 1000 - 500.0f) / 500.0f;
				posY = y * averageDistanceBetweenCubes - height + 1 + (rand() % 1000 - 500.0f) / 500.0f;
				posZ = z * averageDistanceBetweenCubes - depth + 1 + (rand() % 1000 - 500.0f) / 500.0f;
				instancePositions[index] = DirectX::XMFLOAT3(posX, posY, posZ);
			}
		}
	}

	return instancePositions;
}

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

	// -------------------INSTANCING--------------------------
	if (m_instancing)
	{
		//Buffer
		int xInstances = (int)cbrt(NUM_INSTANCE_CUBES);
		int yInstances = xInstances;
		int zInstances = xInstances;
		DirectX::XMFLOAT3* instancePositions = createInstances(xInstances, yInstances, zInstances);
		m_instanceBufferResource = CreateDefaultBuffer(Renderer::getInstance()->getCopyCommandList(), instancePositions, sizeof(DirectX::XMFLOAT3) * m_nrOfInstances, m_instanceUploadHeap, L"Instance heap", D3D12_RESOURCE_STATE_COMMON);

		//View
		m_instanceBufferView.BufferLocation = m_instanceBufferResource->GetGPUVirtualAddress();
		m_instanceBufferView.StrideInBytes = sizeof(DirectX::XMFLOAT3);
		m_instanceBufferView.SizeInBytes = sizeof(DirectX::XMFLOAT3) * m_nrOfInstances;

		delete[] instancePositions;
	}
}

void VertexBuffer::bind()
{
	if (Input::getInstance()->keyPressed(DirectX::Keyboard::Keys::D1))
		m_nrOfInstances = 4096;
	else if (Input::getInstance()->keyPressed(DirectX::Keyboard::Keys::D2))
		m_nrOfInstances = 32768;
	else if (Input::getInstance()->keyPressed(DirectX::Keyboard::Keys::D3))
		m_nrOfInstances = 262144;


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
