#pragma once
#include "../State.h"
class CubeState : public State {
private:
	//Scene
public:
	CubeState();
	~CubeState();

	void initialise();
	void update();
	void record();
	void executeList();
};