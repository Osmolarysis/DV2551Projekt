#pragma once
#include "../State.h"
#include "../../Camera/Camera.h"
#include <DirectXMath.h>
#include "../../Mesh/MeshGroup.h"

class CubeState : public State {
private:
	//Scene
	std::unique_ptr<Camera> m_camera;
	std::vector<MeshGroup> m_meshGroups;

public:
	CubeState();
	~CubeState();

	void initialise();
	void update();
	void record();
	void executeList();
};