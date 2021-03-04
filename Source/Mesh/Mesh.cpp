#include "Mesh.h"
#include "VertexBuffer.h"	


Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	for (auto g : geometryBuffers) {	// tagen från assignment 1.
		g.second.buffer->decRef();
	}
}

void Mesh::addIAVertexBufferBinding(VertexBuffer* buffer, size_t offset, size_t numElements, size_t sizeElement, Location inputStream)
{
	buffer->incRef();
	geometryBuffers[inputStream] = { sizeElement, numElements, offset, buffer };

}

void Mesh::bindIAVertexBuffer(Location location)
{
	// no checking if the key is valid...TODO
	const VertexBufferBind& vb = geometryBuffers[location];
	vb.buffer->bind(vb.offset, vb.numElements * vb.sizeElement, location);
}
