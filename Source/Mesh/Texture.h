#pragma once
#include "..\Renderer\Renderer.h"


class Texture
{
private:
	std::string m_name;
	std::string m_fileName;
	int m_nrOfImages;

	ComPtr<ID3D12Resource2> m_resource[NUM_SWAP_BUFFERS];
	ComPtr<ID3D12Resource2> m_uploadHeap[NUM_SWAP_BUFFERS];

	bool m_isAnimation = false;
public:
	Texture() {};
	Texture(std::string FileNames, int nrOfImages = 1);
	~Texture();

	bool setTexture(std::string FileName, int frameIndex);
	void updateShaderResourceView(int frameIndex);
};