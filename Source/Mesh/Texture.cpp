#include "Texture.h"
#include "..\DXUtility\DXUtility.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\..\Include\stb_image.h"

Texture::Texture(std::string FileNames, int nrOfImages)
{
	m_fileName = FileNames;
	m_nrOfImages = nrOfImages;

	int w, h, bpp;
	unsigned char* rgb = stbi_load(FileNames.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	if (rgb == nullptr)
	{
		fprintf(stderr, "Error loading texture file: %s\n", FileNames.c_str());
	}
	for (int i = 0; i < 2; ++i)
	{
		m_resource[i] = CreateDefaultTexture(Renderer::getInstance()->getCopyCommandList(), &rgb[0], w * h * bpp, m_uploadHeap[i], L"Texture resource", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, w, h, bpp);
		updateShaderResourceView(i);
	}
	delete rgb;
}

Texture::~Texture()
{
}

bool Texture::setTexture(std::string fileName, int frameIndex)
{
	int w, h, bpp;
	unsigned char* rgb = stbi_load(fileName.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	// might want to compare with DirectX::CreateDDSTextureFromFile12 at some point
	if (rgb == nullptr)
	{
		printf("Error loading texture file: %s\n", fileName.c_str());
		return false;
	}

	setUploadHeapData(m_uploadHeap[frameIndex], rgb, w * h * sizeof(unsigned char) * bpp);


	// not done, 

	delete rgb;
	return true;
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
