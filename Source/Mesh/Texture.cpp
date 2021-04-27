#include "Texture.h"
#include "..\DXUtility\DXUtility.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\..\Include\stb_image.h"
#include "..\Utility\Input.h"
const int pixelSize = 4;

void Texture::loadAllImages()
{
	int w, h, bpp;
	unsigned char* rgb;
	std::string filename;

	for (int i = 0; i < m_nrOfImages; ++i)
	{
		filename = m_fileName + std::to_string(i) + m_fileEnding;

		rgb = stbi_load(filename.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
		if (rgb == nullptr)
		{
			printf("Error loading texture file: %s\n", filename.c_str());
		}
		m_pictureData.push_back(rgb);
	}
	m_textureWidth = w;
	m_textureHeight = h;
}

Texture::Texture(std::string FileNames, std::string fileEnding, int nrOfImages)
{
	m_fileName = FileNames;
	m_nrOfImages = nrOfImages;
	m_fileEnding = fileEnding;

	std::string filename = FileNames;
	if (nrOfImages > 1)
	{
		filename = m_fileName + std::to_string(0) + m_fileEnding;
		loadAllImages();
		for (int i = 0; i < 2; ++i)
		{
			m_resource[i] = CreateDefaultTexture(Renderer::getInstance()->getCopyCommandList(), m_pictureData[0], m_textureWidth * m_textureHeight * pixelSize, m_uploadHeap[i], L"Texture resource", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, m_textureWidth, m_textureHeight, pixelSize);
			updateShaderResourceView(i);
		}
	}
	else
	{
		int w, h, bpp;
		unsigned char* rgb = stbi_load(filename.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
		if (rgb == nullptr)
		{
			printf("Error loading texture file: %s\n", filename.c_str());
		}
		for (int i = 0; i < 2; ++i)
		{
			m_resource[i] = CreateDefaultTexture(Renderer::getInstance()->getCopyCommandList(), &rgb[0], w * h * pixelSize, m_uploadHeap[i], L"Texture resource", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, w, h, pixelSize);
			updateShaderResourceView(i);
		}
	}
}

Texture::~Texture()
{
	for (auto el : m_pictureData)
	{
		delete el;
	}
}

bool Texture::loadTexture(std::string fileName, int frameIndex)
{
	int w, h, bpp;
	unsigned char* rgb = stbi_load(fileName.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	// might want to compare with DirectX::CreateDDSTextureFromFile12 at some point
	if (rgb == nullptr)
	{
		printf("Error loading texture file: %s\n", fileName.c_str());
		return false;
	}

	// Update data in default heap
	// describe data
	D3D12_SUBRESOURCE_DATA sData = {};
	sData.pData = rgb;
	sData.RowPitch = w * pixelSize;
	sData.SlicePitch = sData.RowPitch * h;

	// helper function to upload data to upload heap and copy to default heap
	UpdateSubresources(Renderer::getInstance()->getCopyCommandList(), m_resource[frameIndex].Get(), m_uploadHeap[frameIndex].Get(), 0, 0, 1, &sData);
	

	delete rgb;
	return true;
}

void Texture::setTexture(int textureIndex, int frameIndex)
{
	// Update data in default heap
	// describe data
	D3D12_SUBRESOURCE_DATA sData = {};
	sData.pData = m_pictureData.at(textureIndex);
	sData.RowPitch = m_textureWidth * pixelSize;
	sData.SlicePitch = sData.RowPitch * m_textureHeight;

	// helper function to upload data to upload heap and copy to default heap
	UpdateSubresources(Renderer::getInstance()->getCopyCommandList(), m_resource[frameIndex].Get(), m_uploadHeap[frameIndex].Get(), 0, 0, 1, &sData);

}

// This *should* create/update the shader resource with new texture
void Texture::updateShaderResourceView(int frameIndex)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_resource[frameIndex].Get()->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	auto renderer = Renderer::getInstance();

	UINT offsetSize = renderer->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(renderer->getCBDescriptorHeap(frameIndex)->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(NUM_CONSTANT_BUFFERS, offsetSize);

	Renderer::getInstance()->getDevice()->CreateShaderResourceView(m_resource[frameIndex].Get(), &srvDesc, hDescriptor);
}

void Texture::updateAnimation(int frameIndex, float dt)
{
	const float normalUpdateFrequency = 1.f / 30;
	
	static float updateFrequency = 0.f;
	if (Input::getInstance()->keyPressed(DirectX::Keyboard::Keys::F))
	{
		updateFrequency = normalUpdateFrequency * !updateFrequency;
		m_animationTimer = 0;
	}

	m_animationTimer += dt;
	if (m_animationTimer > updateFrequency)
	{
		m_animatedLastFrame = true;
		m_animationTimer -= updateFrequency;

		m_currentAnimationFrame++;
		m_currentAnimationFrame %= m_nrOfImages;

		std::string filename = m_fileName + std::to_string(m_currentAnimationFrame) + m_fileEnding;
		setTexture(m_currentAnimationFrame, frameIndex);
		updateShaderResourceView(frameIndex);

	}
	else if (m_animatedLastFrame)
	{
		m_animatedLastFrame = false;

		std::string filename = m_fileName + std::to_string(m_currentAnimationFrame) + m_fileEnding;
		setTexture(m_currentAnimationFrame, frameIndex);
		updateShaderResourceView(frameIndex);
	}
}
