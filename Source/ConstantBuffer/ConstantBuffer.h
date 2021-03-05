#pragma once

#include "../Renderer/Renderer.h"

class ConstantBuffer 
{
private:
	ComPtr<ID3D12Resource1> m_constantBufferResource[NUM_SWAP_BUFFERS];
	BYTE* m_mappedData[NUM_SWAP_BUFFERS];
	UINT m_bufferSize = 0;
	UINT m_location = 0;

public:
	ConstantBuffer(UINT elementSize, UINT location);
	~ConstantBuffer();

	void setData(const void* data);
	void updateData(const void* data, UINT currentBackBufferIndex);
	void bind();
};