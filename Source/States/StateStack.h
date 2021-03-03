#pragma once
#include "State.h"
#include <WRL/client.h>
#include <vector>

class StateStack {
private:
	static StateStack m_this;
	std::vector<State*> m_stack;

public:
	static StateStack* getInstance();

	//Update & rendering
	void update();
	void record();
	void executeList();

	//Stack functions
	void push(State::StateType); //Add a state to the stack
	void pop(); //Return to previous state
	
	bool isEmpty() const;
};