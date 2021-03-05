#pragma once
#include "Mesh.h"


// A group of meshes that share the same shader and exists wihtin the same scene.
class MeshGroup
{
private:
	std::vector<Mesh*> m_meshes;
		
	ComPtr<ID3D12PipelineState> m_pipelineStateObject;

	// May be moved to some sort of material? probably not
	ComPtr<ID3DBlob> m_vsBlob;
	ComPtr<ID3DBlob> m_psBlob;

public:
	enum class ShaderType {VS = 0, PS = 1, GS = 2, CS = 3};

	MeshGroup(LPCWSTR shaderFiles[]);	// for now assume sending in VS and PS in that order
	~MeshGroup();

	void addMesh(Mesh* mesh);
	void drawAll();

	ComPtr<ID3DBlob> compileShader(LPCWSTR shaderFile, std::string& errString, ShaderType type);
	bool makePipelineStateObject();
};

