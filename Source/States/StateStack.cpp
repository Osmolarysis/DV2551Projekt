#include "StateStack.h"
#include "StateTypes\CubeState.h"

StateStack StateStack::m_this;

State* StateStack::currentState() const { return m_stack.back(); }

StateStack::~StateStack()
{
	printf("Destroying StateStack...\n");
	for (size_t i = 0; i < m_stack.size(); i++)
	{
		printf("\t");
		delete m_stack.at(i);
	}
	printf("Done\n");
}

StateStack* StateStack::getInstance()
{
	return &m_this;
}

void StateStack::update()
{
	if (!m_stack.empty()) {
		currentState()->update();
	}
}

void StateStack::record()
{
	if (!m_stack.empty()) {
		currentState()->record();
	}
}

void StateStack::executeList()
{
	if (!m_stack.empty()) {
		currentState()->executeList();
	}
}

void StateStack::push(State::StateType stateType)
{
	State* newState = nullptr;
	switch (stateType)
	{
	case State::StateType::cubeState:
		newState = new CubeState;
		break;
	default:
		break;
	}
	if (newState != nullptr) {
		//Add new state
		m_stack.push_back(newState);
		//Initialise that state
		m_stack.back()->initialise();
	}
}

void StateStack::pop()
{
	if (!m_stack.empty()) {
		m_stack.pop_back();
	}
}

bool StateStack::isEmpty() const
{
	return m_stack.empty();
}
