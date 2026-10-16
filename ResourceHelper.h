#pragma once

#include <cstddef>
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

class ResourceHelper {
public:
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(ID3D12Device* device, size_t sizeInBytes);

	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorCount, bool shaderVisible);

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
};