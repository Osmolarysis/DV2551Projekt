#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(UINT bufferSize)
{
	m_bufferSize = bufferSize;

	UINT cbSizeAligned = (m_bufferSize + 255) & ~255;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1; //used when multi-gpu (we don't do that here)
	heapProperties.VisibleNodeMask = 1; //used when multi-gpu (we don't do that here)
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		HRESULT hr = Renderer::getInstance()->getDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBufferResource[i])
		);
		if (hr != S_OK) {
			printf("Error creating constant buffer");
		}

		//Mapping data like this means we have to keep track of when it's updated so it's not in use by the GPU
		hr = m_constantBufferResource[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData[i])); 
		if (hr != S_OK) {
			printf("Error mapping constant buffer");
		}
	}
}

ConstantBuffer::~ConstantBuffer()
{
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++) 
	{
		if (m_constantBufferResource[i] != nullptr) {
			m_constantBufferResource[i]->Unmap(0, nullptr);
			m_mappedData[i] = nullptr;
		}
		SafeRelease(m_constantBufferResource[i].GetAddressOf());
	}
}

//Set data for both CB resources and keep it there.
void ConstantBuffer::setData(const void* data)
{
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		memcpy(&m_mappedData[i], data, m_bufferSize);
	}
}

//Set data for current backbuffer index.
void ConstantBuffer::updateData(const void* data, UINT currentBackBufferIndex)
{
	memcpy(&m_mappedData[currentBackBufferIndex], data, m_bufferSize);
}

void ConstantBuffer::bind()
{
	//We need a constant buffer view somewhere that connects the constant buffer to the descriptor heap at the correct location.
	//Should be doable once in the constructor though so this function might not be necessary.
}
