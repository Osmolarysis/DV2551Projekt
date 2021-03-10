#include "Mesh.h"


Mesh::Mesh()
{
}

Mesh::~Mesh()
{

}

void Mesh::addIAVertexBufferBinding(std::shared_ptr<VertexBuffer> vb)
{
	m_vertexBuffer = vb;
}

void Mesh::draw()
{
	// bind IA, cbuffers, etc.
	bindAll(); 

	// draw
	m_vertexBuffer->draw();
}

const Transform* Mesh::getTransform()
{
	return &m_transform;
}

void Mesh::bindIAVertexBuffer()
{
	m_vertexBuffer->bind();
}

void Mesh::bindAll()
{
	bindIAVertexBuffer();

	// cbuffer binds in meshgroup instead

	// bind eventual textures.

	// alternatively if meshes later will have different vertexbuffers, have an array of locations and call bindIA.. with for each.
}


