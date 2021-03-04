#include "MeshGroup.h"

MeshGroup::MeshGroup()
{
}

MeshGroup::~MeshGroup()
{
	SafeRelease(m_pipelineStateObject.GetAddressOf());
}

void MeshGroup::addMesh(Mesh* mesh)
{
	m_meshes.push_back(mesh);
}

void MeshGroup::drawAll()
{
	// bind pipeline state object
	Renderer::getInstance()->getGraphicsCommandList()->SetPipelineState(m_pipelineStateObject.Get());

	// draw all meshes
	for (auto& mesh : m_meshes)
	{
		mesh->draw();
	}
}
