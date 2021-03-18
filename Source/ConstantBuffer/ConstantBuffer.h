#pragma once

#include "../Renderer/Renderer.h"

#define LOCATION_CAMERA 0
#define LOCATION_TRANSLATION 1

class ConstantBuffer 
{
private:
	ComPtr<ID3D12Resource2> m_constantBufferResource[NUM_SWAP_BUFFERS];
	ComPtr<ID3D12Resource2> m_uploadBufferResource[NUM_SWAP_BUFFERS];
	UINT m_bufferSize = 0;
	UINT m_location = 0;

public:
	ConstantBuffer(UINT elementSize, UINT location);
	~ConstantBuffer();

	void setData(const void* data);
	void updateData(const void* data, UINT currentBackBufferIndex, ID3D12GraphicsCommandList* cmdList);
};