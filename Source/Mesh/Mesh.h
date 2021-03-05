#pragma once
#include <unordered_map>
#include "../ConstantBuffer/ConstantBuffer.h"	

class VertexBuffer;		// forward declaration. include in cpp

enum Location { POSITION, COLOR };

// Holds an objects mesh (vertex data) and transform data
// For now every mesh holds vertexdata, even if they are identical. We change so this holds a vector of "objects" that hold only constantbufferdata.
class Mesh
{
private:
	ConstantBuffer* m_cbuffer; // eller kanske bara data.
	// Transform data

	struct VertexBufferBind {
		size_t sizeElement, numElements, offset;
		VertexBuffer* buffer;
	};

	std::unordered_map<Location, VertexBufferBind> m_geometryBuffers;

	// helper functions
	void bindIAVertexBuffer(Location location);
	void bindAll();
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

	void draw();
};

