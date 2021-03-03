#pragma once
class State {
public:
	enum StateType {
		cubeState
	};
	State(StateType type);

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void record() = 0;
	virtual void executeList() = 0;
private:

protected:
	StateType m_stateType;

};