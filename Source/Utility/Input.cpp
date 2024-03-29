#include "Input.h"

Input Input::m_this;

Input::Input() {}

Input::~Input() {}

Input* Input::getInstance() { return &m_this; }

void Input::initialise(HWND window) {
	Input* input = Input::getInstance();

	if (!input->m_isLoaded) {
		input->m_keyboard = std::make_unique<DirectX::Keyboard>(DirectX::Keyboard());
		input->m_mouse = std::make_unique<DirectX::Mouse>(DirectX::Mouse());
		input->m_mouse->SetWindow(window);
		input->m_scrollWheelTracker = 0;
		input->update();
		input->m_isLoaded = true;
		input->m_oldX = input->m_mouseState.x;
		input->m_oldY = input->m_mouseState.y;
	}
}

void Input::update() {
	// Get old mouse coordinated before updating states
	m_oldX = m_mouseState.x;
	m_oldY = m_mouseState.y;

	// Update current state
	m_keyboardState = m_keyboard->GetState();
	m_mouseState = m_mouse->GetState();

	// Track change in scrollwheel
	if (m_scrollWheelTracker == m_mouseState.scrollWheelValue)
		m_scrollDirection = ScrollTracking::STILL;
	else if (m_scrollWheelTracker < m_mouseState.scrollWheelValue)
		m_scrollDirection = ScrollTracking::DOWN;
	else
		m_scrollDirection = ScrollTracking::UP;

	m_keyboardTracker.Update(m_keyboardState);
	m_mouseTracker.Update(m_mouseState);
	m_scrollWheelTracker = m_mouseState.scrollWheelValue;
}

bool Input::keyPressed(DirectX::Keyboard::Keys key) { return m_keyboardTracker.IsKeyPressed(key); }

bool Input::keyReleased(DirectX::Keyboard::Keys key) { return m_keyboardTracker.IsKeyReleased(key); }

bool Input::keyUp(DirectX::Keyboard::Keys key) { return m_keyboardState.IsKeyUp(key); }

bool Input::keyDown(DirectX::Keyboard::Keys key) { return m_keyboardState.IsKeyDown(key); }

bool Input::mousePressed(MouseButton button)
{
	switch (button) {
	case LEFT:
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	case RIGHT:
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	case MIDDLE:
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	}
	return false;
}

bool Input::mouseReleased(MouseButton button)
{
	switch (button) {
	case LEFT:
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	case RIGHT:
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	case MIDDLE:
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	}
	return false;
}

bool Input::mouseDown(MouseButton button)
{
	switch (button) {
	case LEFT:
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::HELD;
	case RIGHT:
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::HELD;
	case MIDDLE:
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::HELD;
	}
	return false;
}

bool Input::mouseUp(MouseButton button)
{
	switch (button) {
	case LEFT:
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::UP;
	case RIGHT:
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::UP;
	case MIDDLE:
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::UP;
	}
	return false;
}

int Input::mouseX() { return m_mouseState.x; }

int Input::mouseY() { return m_mouseState.y; }

int Input::scrollWheelValue() { return m_mouseState.scrollWheelValue; }

bool Input::scrolledUp() { return m_scrollDirection == ScrollTracking::DOWN; }

bool Input::scrolledDown() { return m_scrollDirection == ScrollTracking::UP; }

int Input::getMouseMovementX() { return m_mouseState.x - m_oldX; }

int Input::getMouseMovementY() { return m_mouseState.y - m_oldY; }

void Input::setMouseModeRelative() {
	if (m_mouseState.positionMode == DirectX::Mouse::MODE_ABSOLUTE)
		m_mouse.get()->SetMode(DirectX::Mouse::MODE_RELATIVE);

	if (m_mouse.get()->IsVisible() == true)
		m_mouse.get()->SetVisible(false);
}

void Input::setMouseModeAbsolute() {
	if (m_mouseState.positionMode != DirectX::Mouse::MODE_ABSOLUTE)
		m_mouse.get()->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	if (m_mouse.get()->IsVisible() == false)
		m_mouse.get()->SetVisible(true);
}

DirectX::Mouse::Mode Input::getMouseMode() { return m_mouseState.positionMode; }
