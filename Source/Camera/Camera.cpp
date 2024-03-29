#include "Camera.h"
#include "../Renderer/Renderer.h"
#include "../Utility/Timer.h"
#include "../Utility/Input.h"

using namespace DirectX;

void Camera::rotatePlayer()
{
	Input* input = Input::getInstance();
	Timer* timer = Timer::getInstance();

	//Update camera only if mousemode is relative
	if (input->getMouseMode() == DirectX::Mouse::MODE_RELATIVE) 
	{
		//Switch mode by left clicking
		if (input->mousePressed(MouseButton::LEFT))
			input->setMouseModeAbsolute();
		
		//Get input and speed
		float rotationSpeed = 1.0f * (float)timer->getDt();
		float deltaX = (float)input->mouseX();
		float deltaY = (float)input->mouseY();

		//If mouse moved update yaw and pitch
		if (deltaX != 0.0f)
			m_cameraYaw -= deltaX * rotationSpeed;
		if (deltaY != 0.0f)
			m_cameraPitch -= deltaY * rotationSpeed;
		m_cameraPitch = min(max(m_cameraPitch, -1.5f), 1.5f);

		//Calculate rotation matrices
		XMMATRIX cameraRotationMatrix = XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0f);
		XMMATRIX rotateYTempMatrix = XMMatrixRotationY(m_cameraYaw);

		XMVECTOR defaultForward = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
		XMVECTOR defaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);

		//Rotate movement vectors
		m_forward = XMVector3TransformCoord(defaultForward, cameraRotationMatrix);
		m_up = XMVector3TransformCoord(m_up, rotateYTempMatrix);
		m_right = XMVector3TransformCoord(defaultRight, cameraRotationMatrix);

		m_viewUpdated = true;
	}
	else 
	{
		if (input->mousePressed(MouseButton::LEFT))
			input->setMouseModeRelative();
		else
			input->setMouseModeAbsolute();
	}
}

void Camera::movePlayer()
{
	Input* input = Input::getInstance();
	Timer* timer = Timer::getInstance();
	float speed = 5.0f * (float)timer->getDt();
	
	if (input->keyDown(Keyboard::Keys::LeftShift))
		speed *= 4.0f;

	if (input->keyDown(Keyboard::Keys::W)) {
		m_eye += m_forward * speed;
		m_viewUpdated = true;
	}
	if (input->keyDown(Keyboard::Keys::S)) {
		m_eye -= m_forward * speed;
		m_viewUpdated = true;
	}
	if (input->keyDown(Keyboard::Keys::A)) {
		m_eye -= m_right * speed;
		m_viewUpdated = true;
	}
	if (input->keyDown(Keyboard::Keys::D)) {
		m_eye += m_right * speed;
		m_viewUpdated = true;
	}

	m_target = m_eye + m_forward;
}

Camera::Camera(bool firstPersonCamera)
{
	Renderer* renderer = Renderer::getInstance();

	m_cameraBuffer = std::make_shared<ConstantBuffer>(ConstantBuffer(sizeof(m_matrices), LOCATION_CAMERA));

	//View matrix
	m_eye = XMVectorSet(0.0f, 0.0f, 5.0f, 1.0f);
	m_target = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	m_up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	m_matrices.m_view = XMMatrixLookAtRH(m_eye, m_target, m_up);

	//Projection matrix
	m_aspectRatio = (float)renderer->getScreenWidth() / (float)renderer->getScreenHeight();
	m_fov = 60.0f * XM_PI / 180.0f;
	m_nearPlane = 0.1f;
	m_farPlane = 300.0f;
	m_matrices.m_proj = XMMatrixPerspectiveFovRH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);

	//Movement vectors
	m_forward = m_target;
	m_right = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);

	m_firstPersonCamera = firstPersonCamera;

	m_matrices.dt = (float)Timer::getInstance()->getDt();
	m_cameraBuffer->setData(&m_matrices);
}

Camera::~Camera()
{
	
}

void Camera::setEye(float x, float y, float z)
{
	m_eye = XMVectorSet(x, y, z, 1.0f);
	m_viewUpdated = true;
}

void Camera::setTarget(float x, float y, float z)
{
	m_target = XMVectorSet(x, y, z, 1.0f);
	m_viewUpdated = true;
}

void Camera::setUp(float x, float y, float z)
{
	m_up = XMVectorSet(x, y, z, 1.0f);
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
	if (m_firstPersonCamera) {
		rotatePlayer();
		movePlayer();
	}

	if (Input::getInstance()->keyPressed(Keyboard::Keys::Q)) {
		m_eye = XMVectorSet(-30.0f, 100.0f, -30.0f, 1.0f);
		m_target = XMVectorSet(-29.0f, 99.0f, -29.0f, 1.0f);
		m_up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		m_viewUpdated = true;
		/*XMFLOAT4 eye;
		XMStoreFloat4(&eye, m_eye);
		std::cout << "Eye: " << eye.x << "," << eye.y << "," << eye.z << "\n";
		XMFLOAT4 target;
		XMStoreFloat4(&target, m_target);
		std::cout << "Target: " << target.x << "," << target.y << "," << target.z << "\n";
		XMFLOAT4 up;
		XMStoreFloat4(&up, m_up);
		std::cout << "Up: " << up.x << "," << up.y << "," << up.z << "\n";*/
	}

	if (m_viewUpdated) {
		m_matrices.m_view = XMMatrixLookAtRH(m_eye, m_target, m_up);
		m_viewUpdated = false;
	}
	if (m_projUpdated) {
		m_matrices.m_proj = XMMatrixPerspectiveFovRH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
		m_projUpdated = false;
	}

	m_matrices.dt = (float)Timer::getInstance()->getDt();
	m_cameraBuffer->updateData(&m_matrices, Renderer::getInstance()->getSwapChain()->GetCurrentBackBufferIndex(), Renderer::getInstance()->getCopyCommandList());
}
