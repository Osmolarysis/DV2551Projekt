#pragma once
#include "Mesh.h"
#include "../ConstantBuffer/ConstantBuffer.h"	


// A group of meshes that share the same shader and exists wihtin the same scene.
class MeshGroup
{
private:
	std::vector<std::shared_ptr<Mesh>> m_meshes;					
	std::unique_ptr<ConstantBuffer> m_cbuffer;
	ComPtr<ID3D12PipelineState> m_pipelineStateObject;

	// May be moved to some sort of material? probably not
	ComPtr<ID3DBlob> m_vsBlob;
	ComPtr<ID3DBlob> m_psBlob;

public:
	enum class ShaderType {VS = 0, PS = 1, GS = 2, CS = 3};

	MeshGroup(LPCWSTR shaderFiles[], UINT cbufferSize, UINT cbufferLocation);	// for now assume sending in VS and PS in that order
	~MeshGroup();

	void addMesh(std::shared_ptr<Mesh> mesh);
	void drawAll();

	ComPtr<ID3DBlob> compileShader(LPCWSTR shaderFile, std::string& errString, ShaderType type);
	bool makePipelineStateObject();
};

