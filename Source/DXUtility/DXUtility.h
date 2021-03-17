#pragma once
#include "..\Renderer\Renderer.h"

ComPtr<ID3D12Resource2> makeBufferHeap(D3D12_HEAP_TYPE type, UINT64 size, LPCWSTR heapName);
void setUploadHeapData(ComPtr<ID3D12Resource2> resource, const void* data, size_t size);


ComPtr<ID3D12Resource2> CreateDefaultBuffer(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource2>& uploadBuffer, LPCWSTR name, D3D12_RESOURCE_STATES resourceStateAfter);

class DXUtility
{
};
