#pragma once
#include "..\Renderer\Renderer.h"


class Texture
{
private:
	std::string m_fileName;
	std::string m_fileEnding;
	int m_nrOfImages;
	int m_currentAnimationFrame = 0;
	float m_animationTimer = 0;
	bool m_animatedLastFrame = false;

	ComPtr<ID3D12Resource2> m_resource[NUM_SWAP_BUFFERS];
	ComPtr<ID3D12Resource2> m_uploadHeap[NUM_SWAP_BUFFERS];

	bool m_isAnimation = false;
public:
	Texture() {};
	Texture(std::string FileNames, std::string fileEnding = ".png", int nrOfImages = 1);
	~Texture();

	bool setTexture(std::string FileName, int frameIndex);
	void updateShaderResourceView(int frameIndex);
	void updateAnimation(int frameIndex, float dt);
};