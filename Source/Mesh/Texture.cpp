#include "Texture.h"
#include "..\DXUtility\DXUtility.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\..\Include\stb_image.h"

Texture::Texture(int texWidth, int texHeight, std::string FileNames, int nrOfImages)
{
	m_fileName = FileNames;


}

Texture::~Texture()
{
}

bool Texture::setTexture(int texWidth, int texHeight, std::string fileName)
{
	int frameIndex = Renderer::getInstance()->getSwapChain()->GetCurrentBackBufferIndex();

	int w, h, bpp;
	unsigned char* rgb = stbi_load(fileName.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	// might want to compare with DirectX::CreateDDSTextureFromFile12 at some point
	if (rgb == nullptr)
	{
		printf("Error loading texture file: %s\n", fileName.c_str());
		return -1;
	}

	return true;
}
