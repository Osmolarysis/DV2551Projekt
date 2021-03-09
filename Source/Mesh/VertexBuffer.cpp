#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(size_t size) // may move content to own function that is called in setData
{
	m_totalSize = size; //in bytes

	//Do initial stuff
	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD; // Not ideal. Should use DEFAULT but this is easier for now. TODO: switch to default
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = m_totalSize;	//Not sure about this
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = Renderer::getInstance()->getDevice()->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vertexBufferResource.GetAddressOf())
	);
	if (hr != S_OK) {
		printf("Error creating vertex buffer\n");
		exit(-1);
	}

	m_vertexBufferResource->SetName(L"vb heap");
}

VertexBuffer::~VertexBuffer()
{
	SafeRelease(m_vertexBufferResource.GetAddressOf());
}

void VertexBuffer::setData(const void* data)	// size and offset is in bytes
{
	//This copies some data to the buffer.
	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 };	// We do not intend to read from this resource on the CPU.
	HRESULT hr = m_vertexBufferResource->Map(0, &range, &dataBegin);
	if (hr != S_OK) {
		printf("Error mapping vertex buffer resources");
		exit(-1);
	}
	memcpy((char*)dataBegin, (char*)data, m_totalSize);
	m_vertexBufferResource->Unmap(0, nullptr);

	//Prepare VB view to be used in bind.
	m_vertexBufferView.BufferLocation = m_vertexBufferResource->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = UINT(sizeof(Vertex));
	m_vertexBufferView.SizeInBytes = UINT(m_totalSize);
}

void VertexBuffer::bind(size_t offset, size_t size)
{
	//Location tells us which vertexbuffer we're setting
	Renderer::getInstance()->getGraphicsCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);

}

void VertexBuffer::unbind()
{
	//Seems to only be a helper function. Might not be necessary.
}

size_t VertexBuffer::getSize()
{
	return m_totalSize;
}
