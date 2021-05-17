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
	std::vector<unsigned char*> m_pictureData;
	int m_textureWidth;
	int m_textureHeight;

	ComPtr<ID3D12Resource2> m_resource[NUM_SWAP_BUFFERS];
	ComPtr<ID3D12Resource2> m_uploadHeap[NUM_SWAP_BUFFERS];

	bool m_isAnimation = false;

	void loadAllImages();
public:
	Texture() {};
	Texture(std::string FileNames, std::string fileEnding = ".png", int nrOfImages = 1);
	~Texture();

	bool loadTexture(std::string FileName, int frameIndex);
	void setTexture(int textureIndex, int frameIndex); // set already loaded texture
	void updateShaderResourceView(int frameIndex);
	void updateAnimation(int frameIndex, float dt);
};