#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(UINT bufferSize, UINT location)
{
	Renderer* renderer = Renderer::getInstance();
	m_bufferSize = bufferSize;
	m_location = location;

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
		HRESULT hr = renderer->getDevice()->CreateCommittedResource(
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

		m_constantBufferResource[i]->SetName(L"cb heap");

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_constantBufferResource[i]->GetGPUVirtualAddress();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = cbSizeAligned;

		auto handle = renderer->getCBDescriptorHeap(i)->GetCPUDescriptorHandleForHeapStart();
		auto offset = renderer->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * m_location;
		handle.ptr += offset;

		renderer->getDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}
}

ConstantBuffer::~ConstantBuffer()
{
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++) 
	{
		SafeRelease(m_constantBufferResource[i].GetAddressOf());
	}
}

//Set data for both CB resources and keep it there.
void ConstantBuffer::setData(const void* data)
{
	void* mappedMem = nullptr;
	D3D12_RANGE readRange = { 0,0 };

	for (int i = 0; i < NUM_SWAP_BUFFERS; i++) {
		if (SUCCEEDED(m_constantBufferResource[i]->Map(0, &readRange, &mappedMem))) {
			memcpy(mappedMem, data, m_bufferSize);
			D3D12_RANGE writeRange = { 0, m_bufferSize };
			m_constantBufferResource[i]->Unmap(0, &writeRange);
		}
	}
}

//Set data for current backbuffer index.
void ConstantBuffer::updateData(const void* data, UINT currentBackBufferIndex)
{
	void* mappedMem = nullptr;
	D3D12_RANGE readRange = { 0,0 };

	if (SUCCEEDED(m_constantBufferResource[currentBackBufferIndex]->Map(0, &readRange, &mappedMem))) {
		memcpy(mappedMem, data, m_bufferSize);
		D3D12_RANGE writeRange = { 0, m_bufferSize };
		m_constantBufferResource[currentBackBufferIndex]->Unmap(0, &writeRange);
	}
}
