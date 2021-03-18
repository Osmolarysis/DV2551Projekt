#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(UINT bufferSize, UINT location)
{
	Renderer* renderer = Renderer::getInstance();
	m_bufferSize = bufferSize;
	m_location = location;

	UINT cbSizeAligned = (m_bufferSize + 255) & ~255;

	D3D12_HEAP_PROPERTIES heapPropertiesUpload = {};
	heapPropertiesUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapPropertiesUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesUpload.CreationNodeMask = 1; //used when multi-gpu (we don't do that here)
	heapPropertiesUpload.VisibleNodeMask = 1; //used when multi-gpu (we don't do that here)
	heapPropertiesUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_HEAP_PROPERTIES heapPropertiesDefault = {};
	heapPropertiesDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapPropertiesDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDefault.CreationNodeMask = 1; //used when multi-gpu (we don't do that here)
	heapPropertiesDefault.VisibleNodeMask = 1; //used when multi-gpu (we don't do that here)
	heapPropertiesDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr;

	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		//Create upload buffer
		hr = renderer->getDevice()->CreateCommittedResource(
			&heapPropertiesUpload,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadBufferResource[i])
		);
		if (hr != S_OK) {
			printf("Error creating upload constant buffer");
		}
		m_uploadBufferResource[i]->SetName(L"CB upload");

		//Create default buffer
		hr = renderer->getDevice()->CreateCommittedResource(
			&heapPropertiesDefault,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_constantBufferResource[i])
		);
		if (hr != S_OK) {
			printf("Error creating default constant buffer");
		}
		m_constantBufferResource[i]->SetName(L"CB default");

		//Describe size and location of constant buffer
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_constantBufferResource[i]->GetGPUVirtualAddress();
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = cbSizeAligned;

		//Calculate where in the descriptor heap the CB should be placed.
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
		SafeRelease(m_uploadBufferResource[i].GetAddressOf());
	}
}

//Set data for both CB resources and keep it there.
void ConstantBuffer::setData(const void* data)
{
	Renderer* renderer = Renderer::getInstance();

	//Describe the data that will be set
	D3D12_SUBRESOURCE_DATA vbData = {};
	vbData.pData = data;
	vbData.RowPitch = m_bufferSize;
	vbData.SlicePitch = vbData.RowPitch;

	//Uses direct lists to set initial values. Use updateData() to set specific buffer and command list
	for (int i = 0; i < NUM_CONSTANT_BUFFERS; i++) {
		UpdateSubresources(renderer->getDirectCommandList(i), m_constantBufferResource[i].Get(), m_uploadBufferResource[i].Get(), 0, 0, 1, &vbData);
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_constantBufferResource[i].Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_COMMON);
		renderer->getDirectCommandList(i)->ResourceBarrier(1, &barrier);
	}
}

//Set data for specific backbuffer index using a given command list.
void ConstantBuffer::updateData(const void* data, UINT currentBackBufferIndex, ID3D12GraphicsCommandList* cmdList)
{
	Renderer* renderer = Renderer::getInstance();

	//Describe the data that will be set
	D3D12_SUBRESOURCE_DATA vbData = {};
	vbData.pData = data;
	vbData.RowPitch = m_bufferSize;
	vbData.SlicePitch = vbData.RowPitch;

	UpdateSubresources(cmdList, m_constantBufferResource[currentBackBufferIndex].Get(), m_uploadBufferResource[currentBackBufferIndex].Get(), 0, 0, 1, &vbData);
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_constantBufferResource[currentBackBufferIndex].Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_COMMON);
	cmdList->ResourceBarrier(1, &barrier);
}
