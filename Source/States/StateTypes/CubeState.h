#pragma once
#include "../State.h"
#include "../../Camera/Camera.h"
#include <DirectXMath.h>
#include "../../Mesh/MeshGroup.h"

class CubeState : public State {
private:
	//Scene
	std::vector<std::unique_ptr<MeshGroup>> m_scene;
	std::unique_ptr<Camera> m_camera;

public:
	CubeState();
	~CubeState();

	void initialise();
	void update();
	void record();
	void executeList();
};