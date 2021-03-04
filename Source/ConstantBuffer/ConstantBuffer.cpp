#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
{
}

ConstantBuffer::~ConstantBuffer()
{
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++) {
		SafeRelease(m_constantBufferResource[i].GetAddressOf());
	}
}
