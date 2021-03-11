#include "Mesh.h"


Mesh::Mesh()
{
	calculateMatrix();
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

const XMMATRIX* Mesh::getMatrix()
{
	if (!m_matrixUpdated)
		calculateMatrix();
	return &m_matrix;
}

void Mesh::setPosition(XMFLOAT3 pos)
{
	m_transform.translate = pos;
	m_matrixUpdated = false;
}

void Mesh::setRotation(XMFLOAT3 rot)
{
	m_transform.rotation = rot;
	m_matrixUpdated = false;
}

void Mesh::move(float length, int axis)
{
	switch (axis) {
	case 0:
		m_transform.translate.x += length;
		break;
	case 1:
		m_transform.translate.y += length;
		break;
	case 2:
		m_transform.translate.z += length;
		break;
	}
	m_matrixUpdated = false;
}

void Mesh::rotate(float angle, int axis)
{
	switch (axis) {
	case 0:
		m_transform.rotation.x += angle;
		break;
	case 1:
		m_transform.rotation.y += angle;
		break;
	case 2:
		m_transform.rotation.z += angle;
		break;
	}
	m_matrixUpdated = false;
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

void Mesh::calculateMatrix()
{
	XMMATRIX mat = XMMatrixIdentity();
	mat = XMMatrixMultiply(XMMatrixRotationRollPitchYaw(m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z), mat);
	mat = XMMatrixMultiply(XMMatrixTranslation(m_transform.translate.x, m_transform.translate.y, m_transform.translate.z), mat);
	m_matrix = mat;
	m_matrixUpdated = true;
}


