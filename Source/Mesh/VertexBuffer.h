#pragma once
#include "../Renderer/Renderer.h"

class VertexBuffer
{
private:
	unsigned int refs = 0;

	size_t m_totalSize;
	ComPtr<ID3D12Resource2> m_vertexBufferResource;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

public:
	VertexBuffer(size_t size);
	~VertexBuffer();

	// just taken from assignment 1
	void setData(const void* data, size_t size, size_t offset, size_t nrOfVertices);
	void bind(size_t offset, size_t size);
	void unbind();
	size_t getSize();
	void incRef() { refs++; };
	void decRef() { if (refs > 0) refs--; };
	inline unsigned int refCount() { return refs; };

	// lissajous points
	typedef union {
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
	} float4;
	typedef union {
		struct { float x, y, z; };
		struct { float r, g, b; };
	} float3;
	struct Vertex {
		float3 pos;
		float4 color;
	};
};

