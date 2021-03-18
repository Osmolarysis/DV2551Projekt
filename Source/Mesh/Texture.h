#pragma once
#include "..\Renderer\Renderer.h"


class Texture
{
private:
	std::string m_name;
	std::string m_fileName;

	ComPtr<ID3D12Resource2> m_resource[NUM_SWAP_BUFFERS];
	ComPtr<ID3D12Resource2> m_uploadHeap[NUM_SWAP_BUFFERS];

	bool m_isAnimation = false;
public:
	Texture() {};
	Texture(int texWidth, int texHeight, std::string FileNames, int nrOfImages = 1);
	~Texture();

	bool setTexture(int texWidth, int texHeight, std::string FileName);
};