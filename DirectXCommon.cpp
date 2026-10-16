#include "DirectXCommon.h"
#include <cassert>

using Microsoft::WRL::ComPtr;

namespace {
ComPtr<ID3D12DescriptorHeap> MakeHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count, bool visible) {
	D3D12_DESCRIPTOR_HEAP_DESC desc{type, count, visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0};
	ComPtr<ID3D12DescriptorHeap> heap;
	assert(SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))));
	return heap;
}
}

void DirectXCommon::Initialize(HWND hwnd, int32_t width, int32_t height) {
#ifdef _DEBUG
	ComPtr<ID3D12Debug1> debug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) { debug->EnableDebugLayer(); debug->SetEnableGPUBasedValidation(TRUE); }
#endif
	ComPtr<IDXGIFactory7> factory;
	assert(SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))));
	ComPtr<IDXGIAdapter4> adapter;
	for (UINT i = 0; factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC3 desc{}; adapter->GetDesc3(&desc);
		if (!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) break;
		adapter.Reset();
	}
	const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	for (D3D_FEATURE_LEVEL level : levels) {
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(&device_)))) { break; }
	}
	assert(device_);
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	assert(SUCCEEDED(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue_))));
	assert(SUCCEEDED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator_))));
	assert(SUCCEEDED(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_))));
	assert(SUCCEEDED(commandList_->Close()));
	DXGI_SWAP_CHAIN_DESC1 swapDesc{};
	swapDesc.Width = width; swapDesc.Height = height; swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.SampleDesc.Count = 1; swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 2; swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	ComPtr<IDXGISwapChain1> baseSwap;
	assert(SUCCEEDED(factory->CreateSwapChainForHwnd(queue_.Get(), hwnd, &swapDesc, nullptr, nullptr, &baseSwap)));
	assert(SUCCEEDED(baseSwap.As(&swapChain_)));
	rtvHeap_ = MakeHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvHeap_ = MakeHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvHeap_ = MakeHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	rtvSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	srvSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{}; rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (UINT i = 0; i < 2; ++i) {
		assert(SUCCEEDED(swapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffers_[i]))));
		rtv_[i] = rtvHeap_->GetCPUDescriptorHandleForHeapStart(); rtv_[i].ptr += static_cast<SIZE_T>(rtvSize_) * i;
		device_->CreateRenderTargetView(backBuffers_[i].Get(), &rtvDesc, rtv_[i]);
	}
	D3D12_RESOURCE_DESC depthDesc{}; depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; depthDesc.Width = width; depthDesc.Height = height;
	depthDesc.DepthOrArraySize = 1; depthDesc.MipLevels = 1; depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; depthDesc.SampleDesc.Count = 1; depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_HEAP_PROPERTIES heap{}; heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_CLEAR_VALUE clear{}; clear.Format = depthDesc.Format; clear.DepthStencil.Depth = 1.0f;
	assert(SUCCEEDED(device_->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear, IID_PPV_ARGS(&depth_))));
	dsv_ = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{}; dsvDesc.Format = depthDesc.Format; dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device_->CreateDepthStencilView(depth_.Get(), &dsvDesc, dsv_);
	assert(SUCCEEDED(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_))));
	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr); assert(fenceEvent_);
	viewport_ = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f}; scissor_ = {0, 0, width, height};
}

void DirectXCommon::BeginFrame() {
	assert(SUCCEEDED(allocator_->Reset())); assert(SUCCEEDED(commandList_->Reset(allocator_.Get(), nullptr)));
	const UINT index = swapChain_->GetCurrentBackBufferIndex();
	D3D12_RESOURCE_BARRIER barrier{}; barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; barrier.Transition.pResource = backBuffers_[index].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT; barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET; barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList_->ResourceBarrier(1, &barrier); commandList_->OMSetRenderTargets(1, &rtv_[index], FALSE, &dsv_);
	const float color[] = {0.1f, 0.25f, 0.5f, 1.0f}; commandList_->ClearRenderTargetView(rtv_[index], color, 0, nullptr); commandList_->ClearDepthStencilView(dsv_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList_->RSSetViewports(1, &viewport_); commandList_->RSSetScissorRects(1, &scissor_);
}

void DirectXCommon::EndFrame() {
	const UINT index = swapChain_->GetCurrentBackBufferIndex(); D3D12_RESOURCE_BARRIER barrier{}; barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; barrier.Transition.pResource = backBuffers_[index].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT; barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList_->ResourceBarrier(1, &barrier); assert(SUCCEEDED(commandList_->Close())); ID3D12CommandList* lists[] = {commandList_.Get()}; queue_->ExecuteCommandLists(1, lists); swapChain_->Present(1, 0); WaitForGpu();
}

void DirectXCommon::WaitForGpu() { ++fenceValue_; queue_->Signal(fence_.Get(), fenceValue_); if (fence_->GetCompletedValue() < fenceValue_) { fence_->SetEventOnCompletion(fenceValue_, fenceEvent_); WaitForSingleObject(fenceEvent_, INFINITE); } }
void DirectXCommon::Finalize() { if (queue_ && fence_) WaitForGpu(); if (fenceEvent_) { CloseHandle(fenceEvent_); fenceEvent_ = nullptr; } }
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::CpuSrv(uint32_t index) const { auto h = srvHeap_->GetCPUDescriptorHandleForHeapStart(); h.ptr += static_cast<SIZE_T>(srvSize_) * index; return h; }
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GpuSrv(uint32_t index) const { auto h = srvHeap_->GetGPUDescriptorHandleForHeapStart(); h.ptr += static_cast<UINT64>(srvSize_) * index; return h; }
ComPtr<ID3D12Resource> DirectXCommon::CreateBuffer(ID3D12Device* device, size_t bytes) { D3D12_HEAP_PROPERTIES heap{}; heap.Type = D3D12_HEAP_TYPE_UPLOAD; D3D12_RESOURCE_DESC desc{}; desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; desc.Width = bytes; desc.Height = 1; desc.DepthOrArraySize = 1; desc.MipLevels = 1; desc.SampleDesc.Count = 1; desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; ComPtr<ID3D12Resource> result; assert(SUCCEEDED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&result)))); return result; }
