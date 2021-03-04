#pragma once

#include "../Renderer/Renderer.h"

class ConstantBuffer 
{
private:
	ComPtr<ID3D12Resource1> m_constantBufferResource[NUM_SWAP_BUFFERS];
	BYTE* m_mappedData = nullptr;
	UINT m_elementByteSize = 0;

public:
	ConstantBuffer();
	~ConstantBuffer();

	void setData();
	void bind();
};