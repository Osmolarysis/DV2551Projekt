#include "Camera.h"
#include "../Renderer/Renderer.h"
#include "../Utility/Timer.h"
#include "../Utility/Input.h"
#include <SimpleMath.h>

#include <iostream>

using namespace DirectX;

void Camera::rotatePlayer()
{
	Input* input = Input::getInstance();
	Timer* timer = Timer::getInstance();
	float dt = (float)timer->getDt();

	float deltaX = 0.0f;
	float deltaY = 0.0f;

	if (input->getMouseMode() == DirectX::Mouse::MODE_RELATIVE) {
		deltaX = (float)input->mouseX();
		deltaY = (float)input->mouseY();

		if (input->mousePressed(MouseButton::LEFT))
			input->setMouseModeAbsolute();
	}
	else {
		if (input->mousePressed(MouseButton::LEFT))
			input->setMouseModeRelative();
		else
			input->setMouseModeAbsolute();
	}

	float rotationSpeed = 1.0f * dt;

	if (deltaX != 0.0f) {
		m_cameraYaw -= deltaX * rotationSpeed;
	}
	if (deltaY != 0.0f) {
		m_cameraPitch -= deltaY * rotationSpeed;
	}

	m_cameraPitch = min(max(m_cameraPitch, -1.5f), 1.5f);

	SimpleMath::Matrix cameraRotationMatrix = XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0f);
	SimpleMath::Vector3 cameraTarget = XMVector3TransformCoord(m_forward, cameraRotationMatrix);
	cameraTarget = XMVector3Normalize(cameraTarget);

	SimpleMath::Matrix rotateYTempMatrix = XMMatrixRotationY(m_cameraYaw);

	SimpleMath::Vector3 defaultForward = SimpleMath::Vector3(0.0f, 0.0f, 1.0f);
	SimpleMath::Vector3 defaultRight = SimpleMath::Vector3(1.0f, 0.0f, 0.0f);

	m_forward = XMVector3TransformCoord(defaultForward, cameraRotationMatrix);
	m_up = XMVector3TransformCoord(m_up, rotateYTempMatrix);
	m_right = XMVector3TransformCoord(defaultRight, cameraRotationMatrix);

	m_viewUpdated = true;
}

void Camera::movePlayer()
{
	Input* input = Input::getInstance();
	Timer* timer = Timer::getInstance();
	float speed = 3.0f * (float)timer->getDt();

	SimpleMath::Vector3 pos = m_eye;
	SimpleMath::Vector3 forward = m_forward;
	SimpleMath::Vector3 right = m_right;

	if (input->keyDown(Keyboard::Keys::W)) {
		pos -= forward * speed;
		m_viewUpdated = true;
	}
	if (input->keyDown(Keyboard::Keys::S)) {
		pos += forward * speed;
		m_viewUpdated = true;
	}
	if (input->keyDown(Keyboard::Keys::A)) {
		pos -= right * speed;
		m_viewUpdated = true;
	}
	if (input->keyDown(Keyboard::Keys::D)) {
		pos += right * speed;
		m_viewUpdated = true;
	}

	m_position = pos;
}

Camera::Camera()
{
	Renderer* renderer = Renderer::getInstance();

	m_cameraBuffer = std::make_shared<ConstantBuffer>(ConstantBuffer(sizeof(m_matrices), LOCATION_CAMERA));

	SimpleMath::Vector3 test = SimpleMath::Vector3(1, 1, 1);

	//View matrix
	m_eye = XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
	m_target = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	m_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_matrices.m_view = XMMatrixLookAtRH(m_eye, m_target, m_up);

	//Projection matrix
	m_aspectRatio = (float)renderer->getScreenWidth() / (float)renderer->getScreenHeight();
	m_fov = 60.0f * XM_PI / 180.0f;
	m_nearPlane = 0.1f;
	m_farPlane = 100.0f;
	m_matrices.m_proj = XMMatrixPerspectiveFovRH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);

	//Movement vectors
	m_position = m_eye;
	m_forward = XMVectorSet(0, 0, 1, 0);
	m_right = XMVectorSet(1, 0, 0, 0);

	m_cameraBuffer->setData(&m_matrices);
}

Camera::~Camera()
{
	
}

void Camera::setEye(float x, float y, float z)
{
	m_eye = XMVectorSet(x, y, z, 0.0f);
	m_viewUpdated = true;
}

void Camera::setTarget(float x, float y, float z)
{
	m_target = XMVectorSet(x, y, z, 0.0f);
	m_viewUpdated = true;
}

void Camera::setUp(float x, float y, float z)
{
	m_up = XMVectorSet(x, y, z, 0.0f);
	m_viewUpdated = true;
}

void Camera::setFov(float fov)
{
	m_fov = fov;
	m_projUpdated = true;
}

void Camera::setAspectRatio(float aspecRatio)
{
	m_aspectRatio = aspecRatio;
	m_projUpdated = true;
}

void Camera::setNearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
	m_projUpdated = true;
}

void Camera::setFarPlane(float farPlane)
{
	m_farPlane = farPlane;
	m_projUpdated = true;
}

void Camera::update()
{
	Timer* timer = Timer::getInstance();

	//Just for testing
	/*if (Input::getInstance()->keyDown(Keyboard::Keys::W)) 
		setFov(m_fov - (float)timer->getDt());
	else if (Input::getInstance()->keyDown(Keyboard::Keys::S))
		setFov(m_fov + (float)timer->getDt());*/

	rotatePlayer();
	movePlayer();
	
	if (m_viewUpdated || m_projUpdated)
	{
		if (m_viewUpdated) {
			m_eye = m_position;
			m_target = m_position - m_forward;
			m_matrices.m_view = XMMatrixLookAtRH(m_eye, m_target, m_up);
			m_viewUpdated = false;
		}
		if (m_projUpdated) {
			m_matrices.m_proj = XMMatrixPerspectiveFovRH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
			m_projUpdated = false;
		}

		m_cameraBuffer->updateData(&m_matrices, Renderer::getInstance()->getSwapChain()->GetCurrentBackBufferIndex());
	}
}
