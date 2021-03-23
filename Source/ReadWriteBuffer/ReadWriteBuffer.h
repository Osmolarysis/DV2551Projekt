#pragma once
#include "../Renderer/Renderer.h"

class ReadWriteBuffer
{
private:
	ComPtr<ID3D12Resource2> m_readWriteBuffer;
};