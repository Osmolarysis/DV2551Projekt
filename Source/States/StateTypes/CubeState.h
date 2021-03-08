#pragma once
#include "../State.h"
#include <DirectXMath.h>
#include "../../Mesh/MeshGroup.h"

class CubeState : public State {
private:
	//Scene
	std::vector<MeshGroup> m_meshGroups;

public:
	CubeState();
	~CubeState();

	void initialise();
	void update();
	void record();
	void executeList();
};