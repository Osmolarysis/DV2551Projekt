#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include <WRL/client.h>
#include <iostream>

using Microsoft::WRL::ComPtr;

template<class Interface>
inline void SafeRelease(
	Interface** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

#define SCREEN_HEIGHT Renderer::getInstance()->getScreenHeight()
#define SCREEN_WIDTH Renderer::getInstance()->getScreenWidth()

const unsigned int NUM_SWAP_BUFFERS = 2; //Number of buffers

const unsigned int NUM_COMMANDLISTS = 2;

class Renderer {
private:
	//Singleton
	static Renderer m_this;
	bool m_isLoaded = false;

	//Debug
	ComPtr<ID3D12Debug3> m_debugController;

	//Window
	HWND m_handle;
	const LPCWSTR m_windowTitle = L"Projekt";
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;

	//Device stuff
	ComPtr<IDXGIFactory7> m_factory;
	ComPtr<ID3D12Device8> m_device;

	//Root signature
	ComPtr<ID3D12RootSignature> m_rootSignature;

	//Swapchain
	ComPtr<IDXGISwapChain4> m_swapChain;

	//Render Target
	ComPtr<ID3D12DescriptorHeap> m_renderTargetHeap;
	ComPtr<ID3D12Resource1> m_renderTargets[NUM_SWAP_BUFFERS];
	UINT m_renderTargetDescriptorSize = 0;

	//Commandqueue/list/allocator
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator[NUM_COMMANDLISTS];
	ComPtr<ID3D12GraphicsCommandList> m_graphicsCommandList[NUM_COMMANDLISTS];

	//Fence and event handle
	ComPtr<ID3D12Fence1> m_fence;
	UINT64 m_fenceValue = 0;
	HANDLE m_eventHandle = nullptr;

	//viewport and rect
	D3D12_VIEWPORT m_viewPort;
	D3D12_RECT m_scissorRect;

	Renderer(int, int);
	~Renderer();

	//Initialisation functions
	bool createWindow();
	bool createDevice();
	bool createDebugMode();
	bool createCommandQueue();
	bool createSwapChain();
	bool createFenceAndEventHandle();
	bool createDescriptorHeap();
	bool createRenderTargets();
	bool createViewportAndScissorRect();
	bool createRootSignature();

public:
	static Renderer* getInstance();

	//Window functions
	unsigned int getScreenWidth() const;
	unsigned int getScreenHeight() const;
};