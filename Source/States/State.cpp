#include "State.h"
#include "iostream"

State::State()
{
	m_stateType = State::StateType::cubeState;
}

State::State(StateType type)
{
	m_stateType = type;
}

State::~State()
{
	printf("Destroying state...\n");
}
