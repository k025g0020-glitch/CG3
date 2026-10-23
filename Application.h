#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include "Audio.h"
#include "Input.h"
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"
#include "GraphicsTypes.h"
#include "Window.h"

class Application {
public:
	int Run(HINSTANCE instance);
private:
	void Initialize(HINSTANCE instance);
	void InitializeRootSignatureAndPipeline();
	void InitializeScene();
	void InitializeParticle();
	void InitializeImGui();
	void Update();
	void Draw();
	void Finalize();
	static constexpr int32_t kWidth = 1280, kHeight = 720;
	Window window_;
	DirectXCommon dx_;
	Input input_;
	DebugCamera debugCamera_;
	Audio audio_;
	GraphicsPipeline pipeline_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_, materialResource_, matrixResource_, lightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_[2];
	D3D12_VERTEX_BUFFER_VIEW vertexView_{};
	Material* material_ = nullptr;
	TransformationMatrix* matrix_ = nullptr;
	DirectionalLight* light_ = nullptr;
	ModelData model_;
	Transform objectTransform_{{1,1,1},{0,0,0},{0,0,0}};
	Transform cameraTransform_{{1,1,1},{0,0,0},{0,0,-10}};
	bool useMonsterBall_ = false, debugCameraActive_ = false;
	BlendMode blendMode_ = BlendMode::None;
	bool drawObject3d_ = false;

	// ---- Particle (Instancing) ----
	static constexpr uint32_t kNumInstance = 10; // インスタンス数
	static constexpr uint32_t kInstancingSrvIndex = 3; // Heapの3番目にInstancing用のSRVを作成する
	GraphicsPipeline particlePipeline_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> particleRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12Resource> particleVertexResource_, particleMaterialResource_, instancingResource_;
	D3D12_VERTEX_BUFFER_VIEW particleVertexView_{};
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_{};
	ModelData particleModel_;
	Material* particleMaterial_ = nullptr;
	TransformationMatrix* instancingData_ = nullptr;
	Transform transforms_[kNumInstance]{};
	uint32_t numInstance_ = kNumInstance; // 実際に描画する数（kNumInstance以下）
};
