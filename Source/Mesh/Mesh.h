#pragma once
#include <unordered_map>
#include "../ConstantBuffer/ConstantBuffer.h"	

class VertexBuffer;		// forward declaration. include in cpp

enum Location { POSITION, COLOR };

class Mesh
{
private:
	ConstantBuffer* m_cbuffer; // eller kanske bara data.

	struct VertexBufferBind {
		size_t sizeElement, numElements, offset;
		VertexBuffer* buffer;
	};

	std::unordered_map<Location, VertexBufferBind> geometryBuffers;

	// helper functions
	void bindIAVertexBuffer(Location location);
public:
	Mesh();
	~Mesh();

	// array of buffers with locations (binding points in shaders)
	void addIAVertexBufferBinding(
		VertexBuffer* buffer,
		size_t offset,
		size_t numElements,
		size_t sizeElement,
		Location inputStream);

	void bindAll();
};

