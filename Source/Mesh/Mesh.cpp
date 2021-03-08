#include "Mesh.h"


Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	m_geometryBuffers.buffer->decRef();
}

void Mesh::addIAVertexBufferBinding(VertexBuffer* buffer, size_t offset, size_t numElements, size_t sizeElement)
{
	buffer->incRef();
	m_geometryBuffers = { sizeElement, numElements, offset, buffer };

}

void Mesh::draw()
{
	// bind IA, cbuffers, etc.
	bindAll(); 

	// draw
	UINT nrOfVertices = m_geometryBuffers.numElements;
	Renderer::getInstance()->getGraphicsCommandList()->DrawInstanced(nrOfVertices, 1, 0, 0);
}

const Transform* Mesh::getTransform()
{
	return &m_transform;
}

void Mesh::bindIAVertexBuffer()
{
	// no checking if the key is valid...TODO
	VertexBufferBind& vb = m_geometryBuffers;
	vb.buffer->bind(vb.offset, vb.numElements * vb.sizeElement);
}

void Mesh::bindAll()
{
	bindIAVertexBuffer();

	// cbuffer binds in meshgroup instead

	// bind eventual textures.

	// alternatively if meshes later will have different vertexbuffers, have an array of locations and call bindIA.. with for each.
}


