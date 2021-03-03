#pragma once
#include <unordered_map>

//#include "VertexBuffer.h"	
//#include "ConstantBuffer.h"	
class VertexBuffer;		// deklarationer tills vi gör faktiska klassen
class ConstantBuffer;

enum Location { POSITION, COLOR };

class Mesh
{
private:
	ConstantBuffer* m_cbuffer;

	struct VertexBufferBind {
		size_t sizeElement, numElements, offset;
		VertexBuffer* buffer;
	};


	std::unordered_map<Location, VertexBufferBind> geometryBuffers;
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

	void bindIAVertexBuffer(Location location);
};

