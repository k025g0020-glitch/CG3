#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>

class DirectXCommon {
public:
	void Initialize(HWND hwnd, int32_t width, int32_t height);
	void BeginFrame();
	void EndFrame();
	void Finalize();
	ID3D12Device* Device() const { return device_.Get(); }
	ID3D12GraphicsCommandList* CommandList() const { return commandList_.Get(); }
	ID3D12DescriptorHeap* SrvHeap() const { return srvHeap_.Get(); }
	uint32_t SrvDescriptorSize() const { return srvSize_; }
	D3D12_CPU_DESCRIPTOR_HANDLE CpuSrv(uint32_t index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GpuSrv(uint32_t index) const;
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(ID3D12Device* device, size_t bytes);
private:
	void WaitForGpu();
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_, srvHeap_, dsvHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers_[2], depth_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_[2]{}, dsv_{};
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissor_{};
	uint32_t rtvSize_ = 0, srvSize_ = 0;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = nullptr;
};
