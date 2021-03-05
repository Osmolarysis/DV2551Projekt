#pragma once
#include "Mesh.h"


// A group of meshes that share the same shader and exists wihtin the same scene.
class MeshGroup
{
private:
	std::vector<Mesh*> m_meshes;

	ComPtr<ID3D12PipelineState> m_pipelineStateObject;

public:
	MeshGroup();
	~MeshGroup();

	void addMesh(Mesh* mesh);
	void drawAll();

	// TODO: add compileshader and other piplineStateObject stuff
};

