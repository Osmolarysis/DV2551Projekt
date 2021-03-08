#include "Mesh.h"
#include "VertexBuffer.h"	


Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	for (auto g : m_geometryBuffers) {	// tagen från assignment 1.
		g.second.buffer->decRef();
	}
}

void Mesh::addIAVertexBufferBinding(VertexBuffer* buffer, size_t offset, size_t numElements, size_t sizeElement, Location inputStream)
{
	buffer->incRef();
	m_geometryBuffers[inputStream] = { sizeElement, numElements, offset, buffer };

}

void Mesh::draw()
{
	// bind IA, cbuffers, etc.
	bindAll(); 

	// draw
	UINT nrOfVertices = (UINT)m_geometryBuffers[POSITION].numElements;
	Renderer::getInstance()->getGraphicsCommandList()->DrawInstanced(nrOfVertices, 1, 0, 0);
}

void Mesh::bindIAVertexBuffer(Location location)
{
	// no checking if the key is valid...TODO
	const VertexBufferBind& vb = m_geometryBuffers[location];
	vb.buffer->bind(vb.offset, vb.numElements * vb.sizeElement, location);
}

void Mesh::bindAll()
{
	bindIAVertexBuffer(POSITION);
	bindIAVertexBuffer(COLOR);

	//m_cbuffer->bind();

	// bind eventual textures.

	// alternatively if meshes later will have different vertexbuffers, have an array of locations and call bindIA.. with for each.
}


