#pragma once
#include "../State.h"
#include "../../Camera/Camera.h"
#include <DirectXMath.h>
#include "../../Mesh/MeshGroup.h"

class CubeState : public State {
private:
	//Scene
	Camera* m_camera = nullptr;
	std::vector<std::unique_ptr<MeshGroup>> m_scene;

public:
	CubeState();
	~CubeState();

	void initialise();
	void update();
	void record();
	void executeList();
};