#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include <WRL/client.h>

using namespace Microsoft::WRL;

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

class Renderer {
private:
	//Singleton
	static Renderer m_this;
	bool m_isLoaded = false;

	//Window
	HWND m_handle;
	const LPCWSTR m_windowTitle = L"ovikigt";

	//Device stuff
	ComPtr<ID3D12Device8> m_device;

	Renderer(int, int);
	~Renderer();

public:
	static void initialise(HWND);
};