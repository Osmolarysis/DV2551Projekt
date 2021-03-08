#include "CubeState.h"
#include "..\..\Renderer\Renderer.h"
#include <iostream>

CubeState::CubeState()
{
	printf("Constructing cubeState...\n"); //For debugging, remove when implementing
}

CubeState::~CubeState()
{
	printf("Destroying cubeState...\n");
	if (m_camera != nullptr)
		delete m_camera;
}

void CubeState::initialise()
{
	printf("Initialising cubeState...\n"); //For debugging, remove when implementing

	m_camera = new Camera();
}

void CubeState::update()
{
	//Update the rotation matrix
	m_camera->update();
}

void CubeState::record()
{
	
}

void CubeState::executeList()
{

}
