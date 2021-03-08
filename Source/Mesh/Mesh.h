#pragma once
#include <unordered_map>

#include "VertexBuffer.h"

struct Transform {
	float translate[3] = { 0,0,0 };
	//float rotate[3] = { 0,0,0 };
};

// Holds an objects mesh (vertex data) and transform data
// For now every mesh holds vertexdata, even if they are identical. We change so this holds a vector of "objects" that hold only constantbufferdata.
class Mesh
{
private:
	// Transform data
	Transform m_transform;

	struct VertexBufferBind {
		size_t sizeElement, numElements, offset;
		VertexBuffer* buffer;
	};

	VertexBufferBind m_geometryBuffers;

	// helper functions
	void bindIAVertexBuffer();
	void bindAll();
public:
	Mesh();
	~Mesh();

	// array of buffers with locations (binding points in shaders)
	void addIAVertexBufferBinding(
		VertexBuffer* buffer,
		size_t offset,
		size_t numElements,
		size_t sizeElement);

	void draw();
	const Transform* getTransform();
};

