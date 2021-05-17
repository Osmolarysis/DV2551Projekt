#pragma once
#include <windows.h>
#include <Keyboard.h>
#include <Mouse.h>

enum ScrollTracking { DOWN, STILL, UP };
enum MouseButton { LEFT, MIDDLE, RIGHT };

class Input {
private:
	Input();
	static Input m_this;

	//Keyboard
	std::unique_ptr<DirectX::Keyboard> m_keyboard = nullptr;
	DirectX::Keyboard::State m_keyboardState;
	DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	//Mouse
	std::unique_ptr<DirectX::Mouse> m_mouse = nullptr;
	DirectX::Mouse::State m_mouseState;
	DirectX::Mouse::ButtonStateTracker m_mouseTracker;	
	ScrollTracking m_scrollDirection;
	int m_scrollWheelTracker;
	int m_oldX;
	int m_oldY;
	
	bool m_isLoaded = false;

public:
	static void initialise(HWND window);
	static Input* getInstance();
	
	~Input();
	
	void update();

	//Input for keyboard
	bool keyPressed(DirectX::Keyboard::Keys key);
	bool keyReleased(DirectX::Keyboard::Keys key);
	bool keyDown(DirectX::Keyboard::Keys key);
	bool keyUp(DirectX::Keyboard::Keys key);

	// Input for mouse
	bool mousePressed(MouseButton button);
	bool mouseReleased(MouseButton button);
	bool mouseDown(MouseButton button);
	bool mouseUp(MouseButton button);
	int mouseX();
	int mouseY();
	int scrollWheelValue();
	bool scrolledUp();
	bool scrolledDown();
	int getMouseMovementX();
	int getMouseMovementY();

	void setMouseModeRelative();
	void setMouseModeAbsolute();
	DirectX::Mouse::Mode getMouseMode();
};