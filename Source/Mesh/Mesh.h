#pragma once
#include <unordered_map>
#include "VertexBuffer.h"
#include "Texture.h"
using namespace DirectX;

struct Transform {
	XMFLOAT3 translate = { 0,0,0 };
	XMFLOAT3 rotation  = { 0,0,0 };	
};

// Holds an objects mesh (vertex data) and transform data
// For now every mesh holds vertexdata, even if they are identical. We change so this holds a vector of "objects" that hold only constantbufferdata.
class Mesh
{
private:
	// Transform data
	Transform m_transform;

	// Transform matrix, will apear in constantbuffer
	XMMATRIX m_matrix;
	bool m_matrixUpdated = false;

	// Texture related
	Texture m_texture;

	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	// helper functions
	void bindIAVertexBuffer();
	void bindAll();

	void calculateMatrix();
public:
	Mesh();
	Mesh(std::shared_ptr<VertexBuffer> vb, std::string FileNames, int nrOfImages = 1);
	~Mesh();

	// array of buffers with locations (binding points in shaders)
	void addIAVertexBufferBinding(std::shared_ptr<VertexBuffer> vb);

	void draw();
	const Transform* getTransform();
	const XMMATRIX* getMatrix();
	void setPosition(XMFLOAT3 pos);
	void setRotation(XMFLOAT3 rot);
	void move(float length, int axis = 0);
	void rotate(float angle, int axis = 1);
};

