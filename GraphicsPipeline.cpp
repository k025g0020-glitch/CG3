#include "GraphicsPipeline.h"
#include <cassert>

void GraphicsPipeline::Initialize(ID3D12Device* device, ID3D12RootSignature* rootSignature, IDxcBlob* vs, IDxcBlob* ps) {
	D3D12_INPUT_ELEMENT_DESC elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = rootSignature;
	desc.InputLayout = {elements, _countof(elements)};
	desc.VS = {vs->GetBufferPointer(), vs->GetBufferSize()};
	desc.PS = {ps->GetBufferPointer(), ps->GetBufferSize()};
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.DepthClipEnable = TRUE;
	desc.DepthStencilState.DepthEnable = TRUE;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.SampleDesc.Count = 1;
	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	for (size_t i = 0; i < states_.size(); ++i) {
		desc.BlendState = MakeBlendDesc(static_cast<BlendMode>(i));
		assert(SUCCEEDED(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&states_[i]))));
	}
}

void GraphicsPipeline::Bind(ID3D12GraphicsCommandList* commandList) const {
	commandList->SetPipelineState(states_[static_cast<size_t>(blendMode_)].Get());
}

D3D12_BLEND_DESC GraphicsPipeline::MakeBlendDesc(BlendMode mode) {
	D3D12_BLEND_DESC desc{};
	auto& target = desc.RenderTarget[0];
	target.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	target.SrcBlendAlpha = D3D12_BLEND_ONE;
	target.DestBlendAlpha = D3D12_BLEND_ZERO;
	target.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	if (mode == BlendMode::None) { target.BlendEnable = FALSE; return desc; }
	target.BlendEnable = TRUE;
	switch (mode) {
	case BlendMode::Normal: target.SrcBlend = D3D12_BLEND_SRC_ALPHA; target.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; target.BlendOp = D3D12_BLEND_OP_ADD; break;
	case BlendMode::Add: target.SrcBlend = D3D12_BLEND_SRC_ALPHA; target.DestBlend = D3D12_BLEND_ONE; target.BlendOp = D3D12_BLEND_OP_ADD; break;
	case BlendMode::Subtract: target.SrcBlend = D3D12_BLEND_SRC_ALPHA; target.DestBlend = D3D12_BLEND_ONE; target.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; break;
	case BlendMode::Multiply: target.SrcBlend = D3D12_BLEND_ZERO; target.DestBlend = D3D12_BLEND_SRC_COLOR; target.BlendOp = D3D12_BLEND_OP_ADD; break;
	case BlendMode::Screen: target.SrcBlend = D3D12_BLEND_INV_DEST_COLOR; target.DestBlend = D3D12_BLEND_ONE; target.BlendOp = D3D12_BLEND_OP_ADD; break;
	default: assert(false); break;
	}
	return desc;
}
