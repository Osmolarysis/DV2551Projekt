#pragma once
#include "../State.h"
class CubeState : public State {
private:
	//Scene
public:
	void init();
	void update();
	void record();
	void executeList();
};