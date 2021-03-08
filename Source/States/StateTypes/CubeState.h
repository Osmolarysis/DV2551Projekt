#pragma once
#include "../State.h"
#include "../../Camera/Camera.h"

class CubeState : public State {
private:
	//Scene
	Camera* m_camera = nullptr;
public:
	CubeState();
	~CubeState();

	void initialise();
	void update();
	void record();
	void executeList();
};