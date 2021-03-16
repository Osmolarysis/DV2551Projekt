#pragma once
#include <Keyboard.h>
#include <Mouse.h>

class Input {
private:
	Input();
	static Input m_this;

	std::unique_ptr<DirectX::Keyboard> m_keyboard = nullptr;

	DirectX::Keyboard::State m_keyboardState;
	DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;
	
	bool m_isLoaded = false;
public:
	static void initialise();
	static Input* getInstance();
	
	~Input();
	
	void update();

	bool keyPressed(DirectX::Keyboard::Keys key);
	bool keyReleased(DirectX::Keyboard::Keys key);
	bool keyDown(DirectX::Keyboard::Keys key);
	bool keyUp(DirectX::Keyboard::Keys key);
};