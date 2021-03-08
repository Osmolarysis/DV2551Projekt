#include "Camera.h"
#include "../Renderer/Renderer.h"

using namespace DirectX;

Camera::Camera()
{
	Renderer* renderer = Renderer::getInstance();

	m_cameraBuffer = new ConstantBuffer(sizeof(m_matrices), LOCATION_CAMERA);

	m_eye = XMVectorSet(2.0f, 2.0f, 2.0f, 0.0f);
	m_target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	m_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_matrices.m_view = XMMatrixLookAtRH(m_eye, m_target, m_up);

	m_aspectRatio = (float)renderer->getScreenWidth() / (float)renderer->getScreenHeight();
	m_fov = 60.0f * XM_PI / 180.0f;
	m_nearPlane = 0.1f;
	m_farPlane = 100.0f;
	m_matrices.m_proj = XMMatrixPerspectiveFovRH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);

	//m_cameraBuffer->setData(&m_matrices);
}

Camera::~Camera()
{
	if (m_cameraBuffer != nullptr)
		delete m_cameraBuffer;
}

void Camera::setEye(float x, float y, float z)
{
	m_eye = XMVectorSet(x, y, z, 0.0f);
}

void Camera::setTarget(float x, float y, float z)
{
	m_target = XMVectorSet(x, y, z, 0.0f);
}

void Camera::setUp(float x, float y, float z)
{
	m_up = XMVectorSet(x, y, z, 0.0f);
}

void Camera::setFov(float fov)
{
	m_fov = fov;
}

void Camera::setAspectRatio(float aspecRatio)
{
	m_aspectRatio = aspecRatio;
}

void Camera::setNearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
}

void Camera::setFarPlane(float farPlane)
{
	m_farPlane = farPlane;
}

void Camera::update()
{
	m_matrices.m_view = XMMatrixLookAtRH(m_eye, m_target, m_up);
	m_matrices.m_proj = XMMatrixPerspectiveFovRH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);

	//m_cameraBuffer->updateData(m_matrices, [insert current backbuffer here])
}
