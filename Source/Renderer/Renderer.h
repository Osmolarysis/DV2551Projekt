#pragma once
#include <windows.h>
#include <initguid.h> //Is needed for d3dx12.h. Some things break if not included before d3d12.h
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include <WRL/client.h>
#include <iostream>
#include <string>

#include "..\..\Include\d3dx12.h"

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
const unsigned int NUM_CONSTANT_BUFFERS = 1;
const unsigned int NUM_TEXTURE_BUFFERS = 1;
const unsigned int NUM_BUFFERS_IN_DESC_HEAP = NUM_CONSTANT_BUFFERS + NUM_TEXTURE_BUFFERS;
const unsigned int NUM_COMMANDLISTS = 2;
const unsigned int NUM_INSTANCE_CUBES = 262144; //64^3
//const unsigned int NUM_INSTANCE_CUBES = 32768; //32^3
//const unsigned int NUM_INSTANCE_CUBES = 4096; //16^3

class Renderer {
private:
	//Singleton
	static Renderer m_this;
	bool m_isLoaded = false;

	//Debug
	ComPtr<ID3D12Debug3> m_debugController;

	//Window
	HWND m_handle;
	std::wstring m_windowTitle = L"Projekt";
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

	//Depth stencil
	ComPtr<ID3D12DescriptorHeap> m_dbDescriptorHeap;
	ComPtr<ID3D12Resource> m_depthStencilBuffer[NUM_SWAP_BUFFERS];
	UINT m_depthBufferDescriptorSize = 0;
	bool dsvSetWrite[NUM_SWAP_BUFFERS] = { false, false };

	//Constant Buffer Descriptor Heaps
	ComPtr<ID3D12DescriptorHeap> m_cbDescriptorHeaps[NUM_SWAP_BUFFERS];

	//SRV descriptor heaps // TODO: make work or remove
	ComPtr<ID3D12DescriptorHeap> m_SRVDescriptorHeaps[NUM_SWAP_BUFFERS];

	//Direct queue/list/allocator
	ComPtr<ID3D12CommandQueue> m_directQueue;
	ComPtr<ID3D12CommandAllocator> m_directAllocator[NUM_COMMANDLISTS];
	ComPtr<ID3D12GraphicsCommandList> m_graphicsDirectList[NUM_COMMANDLISTS];

	//Copy queue/list/allocator
	ComPtr<ID3D12CommandQueue> m_copyQueue;
	ComPtr<ID3D12CommandAllocator> m_copyAllocator[NUM_COMMANDLISTS];
	ComPtr<ID3D12GraphicsCommandList> m_graphicsCopyList[NUM_COMMANDLISTS];

	//Compute queue/list/allocator
	ComPtr<ID3D12CommandQueue> m_computeQueue;
	ComPtr<ID3D12CommandAllocator> m_computeAllocator[NUM_COMMANDLISTS];
	ComPtr<ID3D12GraphicsCommandList> m_graphicsComputeList[NUM_COMMANDLISTS];

	//Fence and event handle
	ComPtr<ID3D12Fence1> m_fence[NUM_SWAP_BUFFERS]; //Fence for syncing queues
	UINT64 m_fenceValue[NUM_SWAP_BUFFERS] = { 0, 0 };
	HANDLE m_eventHandle[NUM_SWAP_BUFFERS] = { nullptr, nullptr };
	UINT64 m_frameComplete[NUM_SWAP_BUFFERS] = { 0, 0 };

	//Game logic fence
	/*ComPtr<ID3D12Fence1> m_gameLogicFence;
	UINT64 m_lastFinishedGameLogicUpdate = 0;*/

	//Copy queue fence for recording
	ComPtr<ID3D12Fence1> m_copyFence;
	UINT64 m_copyFenceValue = 0;
	HANDLE m_copyHandle = nullptr;
	HANDLE m_copyThreadHandle = nullptr;

	//Compute queue fence for recording
	ComPtr<ID3D12Fence1> m_computeFence;
	UINT64 m_computeFenceValue = 0;
	HANDLE m_computeHandle = nullptr;
	HANDLE m_computeThreadHandle = nullptr;

	//Direct queue fence for recording
	ComPtr<ID3D12Fence1> m_directFence;
	UINT64 m_directFenceValue = 0;
	HANDLE m_directHandle = nullptr;
	HANDLE m_directThreadHandle = nullptr;

	//viewport and rect
	D3D12_VIEWPORT m_viewPort;
	D3D12_RECT m_scissorRect;

	//Timestamp

	UINT64* m_copyTimeGPUStart[2];
	UINT64* m_copyTimeCPUStart[2];
	UINT64* m_copyTimeGPUEnd[2];
	UINT64* m_copyTimeCPUEnd[2];
	UINT64* m_copyTimeFrequency;
		  
	UINT64* m_computeTimeGPUStart[2];
	UINT64* m_computeTimeCPUStart[2];
	UINT64* m_computeTimeGPUEnd[2];
	UINT64* m_computeTimeCPUEnd[2];
	UINT64* m_computeTimeFrequency;
		  
	UINT64* m_directTimeGPUStart[2];
	UINT64* m_directTimeCPUStart[2];
	UINT64* m_directTimeGPUEnd[2];
	UINT64* m_directTimeCPUEnd[2];
	UINT64* m_directTimeFrequency;

	Renderer(int, int);
	~Renderer();

	//Initialisation functions
	bool createWindow();
	bool createDevice();
	bool createDebugMode();
	bool createCommandQueues();
	bool createDirectQueue();
	bool createCopyQueue();
	bool createComputeQueue();
	bool createSwapChain();
	bool createFenceAndEventHandle();
	bool createDescriptorHeap();
	bool createRenderTargets();
	bool createDepthStencil();
	bool createViewportAndScissorRect();
	bool createRootSignature();
	bool InitialiseTimestamps();

public:
	//Gets
	static Renderer* getInstance();
	ID3D12Device8* getDevice();
	ID3D12RootSignature* getRootSignature();
	IDXGISwapChain4* getSwapChain();
	HWND getWindowHandle();

	ID3D12Fence1* getCopyFence();
	UINT64 incAndGetCopyValue();
	UINT64 getCopyValue();
	HANDLE getCopyHandle();
	HANDLE getCopyThreadHandle();
	ID3D12GraphicsCommandList* getCopyCommandList();
	ID3D12GraphicsCommandList* getCopyCommandList(int);
	ID3D12CommandAllocator* getCopyCommandAllocator(int);

	ID3D12Fence1* getComputeFence();
	UINT64 incAndGetComputeValue();
	UINT64 getComputeValue();
	HANDLE getComputeHandle();
	HANDLE getComputeThreadHandle();
	ID3D12GraphicsCommandList* getComputeCommandList();
	ID3D12GraphicsCommandList* getComputeCommandList(int);
	ID3D12CommandAllocator* getComputeCommandAllocator(int);

	ID3D12Fence1* getDirectFence();
	UINT64 incAndGetDirectValue();
	UINT64 getDirectValue();
	HANDLE getDirectHandle();
	HANDLE getDirectThreadHandle();
	ID3D12GraphicsCommandList* getDirectCommandList();
	ID3D12GraphicsCommandList* getDirectCommandList(int bufferIndex);
	ID3D12CommandAllocator* getDirectCommandAllocator(int);

	D3D12_VIEWPORT* getViewPort();
	D3D12_RECT* getScissorRect();
	ID3D12Resource1* getRenderTarget(int);
	ID3D12DescriptorHeap* getRenderTargetHeap();
	size_t getRenderTargetHeapSize();
	ID3D12DescriptorHeap* getDepthBufferHeap();
	size_t getDepthBufferHeapSize();
	ID3D12DescriptorHeap* getConstantBufferHeap(int);

	void closeCommandLists();

	//Window functions
	unsigned int getScreenWidth() const;
	unsigned int getScreenHeight() const;

	//Rendering functions
	void beginFrame();
	void executeList();
	void present();

	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	//Utility
	void setWindowTitle(std::wstring);
	void waitForGPU();

	//Descriptor heap functions
	ID3D12DescriptorHeap* getCBDescriptorHeap(UINT bufferIndex);
	ID3D12DescriptorHeap* getSRVDescriptorHeap(UINT bufferIndex);

	bool isDeveloperModeOn();
};