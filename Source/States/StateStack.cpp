#include "StateStack.h"

StateStack StateStack::m_this;

StateStack* StateStack::getInstance()
{
	return &m_this;
}

void StateStack::push(State::StateType)
{
}
