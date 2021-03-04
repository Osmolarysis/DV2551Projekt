#pragma once
class State {
public:
	enum class StateType {
		cubeState = 0,
		nrOfStates = 1
	};
	State();
	State(StateType type);
	virtual ~State();

	virtual void initialise() = 0;
	virtual void update() = 0;
	virtual void record() = 0;
	virtual void executeList() = 0;
private:

protected:
	StateType m_stateType;

};