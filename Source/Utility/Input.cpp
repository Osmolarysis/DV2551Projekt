#include "Input.h"

Input Input::m_this;

Input::Input() {}

Input::~Input() {}

Input* Input::getInstance() { return &m_this; }

void Input::initialise() {
	Input* input = Input::getInstance();

	if (!input->m_isLoaded) {
		input->m_keyboard = std::make_unique<DirectX::Keyboard>(DirectX::Keyboard());
		input->update();
		input->m_isLoaded = true;
	}
}

void Input::update() {
	m_keyboardState = m_keyboard->GetState();
	m_keyboardTracker.Update(m_keyboardState);
}

bool Input::keyPressed(DirectX::Keyboard::Keys key) { return m_keyboardTracker.IsKeyPressed(key); }

bool Input::keyReleased(DirectX::Keyboard::Keys key) { return m_keyboardTracker.IsKeyReleased(key); }

bool Input::keyUp(DirectX::Keyboard::Keys key) { return m_keyboardState.IsKeyUp(key); }

bool Input::keyDown(DirectX::Keyboard::Keys key) { return m_keyboardState.IsKeyDown(key); }
