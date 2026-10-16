#include "ResourceHelper.h"

#include <cassert>

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12Resource> ResourceHelper::CreateBuffer(ID3D12Device* device, size_t sizeInBytes) {

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ComPtr<ID3D12Resource> resource;

	const HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));

	assert(SUCCEEDED(hr));

	return resource;
}

ComPtr<ID3D12DescriptorHeap> ResourceHelper::CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorCount, bool shaderVisible) {

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = type;
	desc.NumDescriptors = descriptorCount;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	const HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));

	assert(SUCCEEDED(hr));

	return descriptorHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE
ResourceHelper::GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += static_cast<SIZE_T>(descriptorSize) * index;

	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE
ResourceHelper::GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();

	handle.ptr += static_cast<UINT64>(descriptorSize) * index;

	return handle;
}