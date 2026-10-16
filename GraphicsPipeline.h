#pragma once
#include <array>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include "GraphicsTypes.h"

class GraphicsPipeline {
public:
	void Initialize(ID3D12Device* device, ID3D12RootSignature* rootSignature, IDxcBlob* vs, IDxcBlob* ps);
	void SetBlendMode(BlendMode mode) { blendMode_ = mode; }
	BlendMode GetBlendMode() const { return blendMode_; }
	void Bind(ID3D12GraphicsCommandList* commandList) const;
private:
	static D3D12_BLEND_DESC MakeBlendDesc(BlendMode mode);
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::Count)> states_{};
	BlendMode blendMode_ = BlendMode::Normal;
};
