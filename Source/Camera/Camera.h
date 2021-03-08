#pragma once

#include <DirectXMath.h>
#include "../ConstantBuffer/ConstantBuffer.h"

class Camera
{
private:
	struct Matrices {
		DirectX::XMMATRIX m_view;
		DirectX::XMMATRIX m_proj;
	};

	ConstantBuffer* m_cameraBuffer;
	Matrices m_matrices;

	DirectX::XMVECTOR m_eye;
	DirectX::XMVECTOR m_target;
	DirectX::XMVECTOR m_up;

	float m_fov;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;
public:
	Camera();
	~Camera();

	void setEye(float x, float y, float z);
	void setTarget(float x, float y, float z);
	void setUp(float x, float y, float z);

	void setFov(float fov);
	void setAspectRatio(float aspecRatio);
	void setNearPlane(float nearPlane);
	void setFarPlane(float farPlane);

	void update();
};