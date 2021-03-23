#pragma once
#include "..\Renderer\Renderer.h"

ComPtr<ID3D12Resource2> makeBufferHeap(D3D12_HEAP_TYPE type, UINT64 size, LPCWSTR heapName);
ComPtr<ID3D12Resource2> makeTextureHeap(D3D12_HEAP_TYPE type, UINT64 size, LPCWSTR heapName, UINT width, UINT height);
void setUploadHeapData(ComPtr<ID3D12Resource2> resource, const void* data, size_t size);
void updateBufferHeap(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource2>& uploadBuffer, D3D12_RESOURCE_STATES resourceStateAfter);

ComPtr<ID3D12Resource2> CreateDefaultBuffer(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource2>& uploadBuffer, LPCWSTR name, D3D12_RESOURCE_STATES resourceStateAfter);
ComPtr<ID3D12Resource2> CreateDefaultTexture(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource2>& uploadBuffer, LPCWSTR name, D3D12_RESOURCE_STATES resourceStateAfter, UINT width, UINT height, UINT pixelSize);

class DXUtility
{
};

