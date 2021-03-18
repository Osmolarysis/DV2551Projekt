#pragma once
#include "../Renderer/Renderer.h"
#include <DirectXMath.h>

class VertexBuffer
{
private:
	unsigned int refs = 0;

	size_t m_totalSize = 0;
	size_t m_totalSizeIndices = 0;
	size_t m_nrOfVertices = 0;
	size_t m_nrOfIndices = 0;
	ComPtr<ID3D12Resource2> m_vertexBufferResource;
	ComPtr<ID3D12Resource2> m_indexBufferResource;
	ComPtr<ID3D12Resource2> m_VBUploadHeap;	// possibly put in some chared vector with fence values to see when ok to release
	ComPtr<ID3D12Resource2> m_IBUploadHeap;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
public:
	VertexBuffer();
	~VertexBuffer();

	// just taken from assignment 1
	void setData(const void* data, size_t dataByteSize, const void* indices = nullptr, size_t indexByteSize = 0);	// vertex and index buffer
	void bind();
	void draw();
	size_t getSize();


	void incRef() { refs++; };
	void decRef() { if (refs > 0) refs--; };
	inline unsigned int refCount() { return refs; };
		
	struct Vertex {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 uv;
		Vertex() {};
		Vertex(const DirectX::XMFLOAT3& p,
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT4& c,
			const DirectX::XMFLOAT2& texC) :
			pos(p),
			normal(n),
			color(c),
			uv(texC) {};
		Vertex(float px, float py, float pz,
			float nx, float ny, float nz,
			float u, float v) :
			pos(px, py, pz),
			normal(nx, ny, nz),
			color(0, 0, 0, 1),
			uv(u, v) {};
	};
};

