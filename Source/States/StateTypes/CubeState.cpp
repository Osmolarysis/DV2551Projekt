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
}

void CubeState::initialise()
{
	printf("Initialising cubeState...\n"); //For debugging, remove when implementing

	m_camera = std::make_unique<Camera>(Camera());
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
