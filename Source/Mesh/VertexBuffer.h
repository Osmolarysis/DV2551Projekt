#pragma once
#include "../Renderer/Renderer.h"
#include <DirectXMath.h>

class VertexBuffer
{
private:
	unsigned int refs = 0;

	size_t m_totalSize = 0;
	size_t m_nrOfVertices = 0;
	size_t m_nrOfIndices = 0;
	ComPtr<ID3D12Resource2> m_vertexBufferResource;
	ComPtr<ID3D12Resource2> m_indexBufferResource;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
public:
	VertexBuffer(size_t size, size_t sizeIndexData = 0);
	~VertexBuffer();

	// just taken from assignment 1
	void setData(const void* data);
	void setData(const void* data, const void* indices);
	void bind();
	void draw();
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
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
	};
};

