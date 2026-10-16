#include "Application.h"
#include <cassert>
#include <cstring>
#include <format>
#include <string>
#include <dxcapi.h>
#include "ConvertString.h"
#include "ModelLoader.h"
#include "externals/DirectXTex/DirectXTex.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

using Microsoft::WRL::ComPtr;

namespace {
ComPtr<IDxcBlob> CompileShader(const wchar_t* path, const wchar_t* profile) {
	ComPtr<IDxcUtils> utils; ComPtr<IDxcCompiler3> compiler; ComPtr<IDxcIncludeHandler> include;
	assert(SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))));
	assert(SUCCEEDED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))));
	assert(SUCCEEDED(utils->CreateDefaultIncludeHandler(&include)));
	ComPtr<IDxcBlobEncoding> source; assert(SUCCEEDED(utils->LoadFile(path, nullptr, &source)));
	DxcBuffer buffer{source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_UTF8};
	LPCWSTR args[] = {path, L"-E", L"main", L"-T", profile, L"-Zi", L"-Qembed_debug", L"-Od", L"-Zpr"};
	ComPtr<IDxcResult> result; assert(SUCCEEDED(compiler->Compile(&buffer, args, _countof(args), include.Get(), IID_PPV_ARGS(&result))));
	ComPtr<IDxcBlobUtf8> errors; result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	if (errors && errors->GetStringLength()) { OutputDebugStringA(errors->GetStringPointer()); assert(false); }
	ComPtr<IDxcBlob> blob; assert(SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr))); return blob;
}

ComPtr<ID3D12Resource> LoadTexture(ID3D12Device* device, const std::string& path, DirectX::TexMetadata& metadata) {
	DirectX::ScratchImage source, mip;
	assert(SUCCEEDED(DirectX::LoadFromWICFile(ConvertString(path).c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, source)));
	assert(SUCCEEDED(DirectX::GenerateMipMaps(source.GetImages(), source.GetImageCount(), source.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mip)));
	metadata = mip.GetMetadata();
	D3D12_RESOURCE_DESC desc{}; desc.Width = metadata.width; desc.Height = static_cast<UINT>(metadata.height); desc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	desc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); desc.Format = metadata.format; desc.SampleDesc.Count = 1; desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	D3D12_HEAP_PROPERTIES heap{}; heap.Type = D3D12_HEAP_TYPE_CUSTOM; heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; heap.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	ComPtr<ID3D12Resource> resource; assert(SUCCEEDED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource))));
	for (size_t level = 0; level < metadata.mipLevels; ++level) { const DirectX::Image* image = mip.GetImage(level, 0, 0); assert(SUCCEEDED(resource->WriteToSubresource(static_cast<UINT>(level), nullptr, image->pixels, static_cast<UINT>(image->rowPitch), static_cast<UINT>(image->slicePitch)))); }
	return resource;
}

void CreateSrv(ID3D12Device* device, ID3D12Resource* resource, const DirectX::TexMetadata& metadata, D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{}; desc.Format = metadata.format; desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; desc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels); device->CreateShaderResourceView(resource, &desc, handle);
}
}

int Application::Run(HINSTANCE instance) {
	Initialize(instance);
	while (window_.ProcessMessage()) { Update(); Draw(); }
	Finalize();
	return 0;
}

void Application::Initialize(HINSTANCE instance) {
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	window_.Initialize(instance, kWidth, kHeight, L"CG3");
	input_.Initialize(instance, window_.GetHwnd());
	debugCamera_.Initialize();
	dx_.Initialize(window_.GetHwnd(), kWidth, kHeight);
	audio_.Initialize(); audio_.LoadAndPlayWave("resources/Alarm01.wav");
	InitializeRootSignatureAndPipeline();
	InitializeScene();
	InitializeImGui();
}

void Application::InitializeRootSignatureAndPipeline() {
	D3D12_DESCRIPTOR_RANGE range{}; range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; range.NumDescriptors = 1; range.BaseShaderRegister = 0; range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	D3D12_ROOT_PARAMETER parameters[4]{};
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; parameters[0].Descriptor.ShaderRegister = 0;
	parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; parameters[1].Descriptor.ShaderRegister = 1;
	parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; parameters[2].DescriptorTable = {1, &range};
	parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; parameters[3].Descriptor.ShaderRegister = 2;
	D3D12_STATIC_SAMPLER_DESC sampler{}; sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; sampler.MaxLOD = D3D12_FLOAT32_MAX; sampler.ShaderRegister = 0; sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	D3D12_ROOT_SIGNATURE_DESC desc{}; desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; desc.NumParameters = _countof(parameters); desc.pParameters = parameters; desc.NumStaticSamplers = 1; desc.pStaticSamplers = &sampler;
	ComPtr<ID3DBlob> blob, error; HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error); if (FAILED(hr)) { if (error) OutputDebugStringA(static_cast<char*>(error->GetBufferPointer())); assert(false); }
	assert(SUCCEEDED(dx_.Device()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_))));
	auto vs = CompileShader(L"Object3d.VS.hlsl", L"vs_6_0"); auto ps = CompileShader(L"Object3d.PS.hlsl", L"ps_6_0"); pipeline_.Initialize(dx_.Device(), rootSignature_.Get(), vs.Get(), ps.Get());
}

void Application::InitializeScene() {
	// fence.obj、fence.mtl、テクスチャを
	// resourcesフォルダーへ配置しておく
	model_ = ModelLoader::LoadObj("resources", "fence.obj");

	vertexResource_ = DirectXCommon::CreateBuffer(dx_.Device(), sizeof(VertexData) * model_.vertices.size());

	vertexView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();

	vertexView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * model_.vertices.size());

	vertexView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertices = nullptr;

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertices));

	std::memcpy(vertices, model_.vertices.data(), sizeof(VertexData) * model_.vertices.size());

	materialResource_ = DirectXCommon::CreateBuffer(dx_.Device(), sizeof(Material));

	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&material_));

	material_->color = {1.0f, 1.0f, 1.0f, 1.0f};

	material_->enableLighting = true;
	material_->uvTransform = calc::MakeIdentity4x4();

	matrixResource_ = DirectXCommon::CreateBuffer(dx_.Device(), sizeof(TransformationMatrix));

	matrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&matrix_));

	matrix_->WVP = calc::MakeIdentity4x4();

	matrix_->World = calc::MakeIdentity4x4();

	matrix_->WorldInverseTranspose = calc::MakeIdentity4x4();

	lightResource_ = DirectXCommon::CreateBuffer(dx_.Device(), sizeof(DirectionalLight));

	lightResource_->Map(0, nullptr, reinterpret_cast<void**>(&light_));

	light_->color = {1.0f, 1.0f, 1.0f, 1.0f};

	light_->direction = {0.0f, -1.0f, 0.0f};

	light_->intensity = 1.0f;

	DirectX::TexMetadata metadata[2]{};

	// 比較表示用
	textureResource_[0] = LoadTexture(dx_.Device(), "resources/uvChecker.png", metadata[0]);

	// fence.mtlのmap_Kdで指定されたテクスチャ
	textureResource_[1] = LoadTexture(dx_.Device(), model_.material.textureFilePath, metadata[1]);

	CreateSrv(dx_.Device(), textureResource_[0].Get(), metadata[0], dx_.CpuSrv(1));

	CreateSrv(dx_.Device(), textureResource_[1].Get(), metadata[1], dx_.CpuSrv(2));
}

void Application::InitializeImGui() {
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui::StyleColorsDark(); ImGui_ImplWin32_Init(window_.GetHwnd());
	ImGui_ImplDX12_Init(dx_.Device(), 2, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, dx_.SrvHeap(), dx_.CpuSrv(0), dx_.GpuSrv(0));
#endif
}

void Application::Update() {
	input_.Update();
	if (input_.TriggerKey(DIK_RETURN)) debugCameraActive_ = !debugCameraActive_;
	if (debugCameraActive_) debugCamera_.Update(input_);
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();
	const char* names[] = {"None", "Normal", "Add", "Subtract", "Multiply", "Screen"}; int selected = static_cast<int>(blendMode_);
	ImGui::Begin("Debug Window"); ImGui::DragFloat3("Camera Position", &cameraTransform_.translate.x, 0.1f); ImGui::DragFloat3("Object Rotation", &objectTransform_.rotate.x, 0.01f);
	ImGui::Checkbox("useMonsterBall", &useMonsterBall_); ImGui::ColorEdit4("Material Color", &material_->color.x); ImGui::ColorEdit4("Light Color", &light_->color.x);
	ImGui::SliderFloat3("Light Direction", &light_->direction.x, -1, 1); ImGui::SliderFloat("Intensity", &light_->intensity, 0, 5);
	if (ImGui::Combo("Blend Mode", &selected, names, IM_ARRAYSIZE(names))) { blendMode_ = static_cast<BlendMode>(selected); pipeline_.SetBlendMode(blendMode_); }
	ImGui::End(); ImGui::Render();
#endif
	const Matrix4x4 world = calc::MakeAffineMatrix(objectTransform_.scale, objectTransform_.rotate, objectTransform_.translate);
	const Matrix4x4 view = debugCameraActive_ ? debugCamera_.GetViewMatrix() : calc::Inverse(calc::MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate));
	const Matrix4x4 projection = debugCameraActive_ ? debugCamera_.GetProjectionMatrix() : calc::MakePerspectiveFovMatrix(0.45f, static_cast<float>(kWidth) / kHeight, 0.1f, 100.0f);
	matrix_->WVP = calc::Multiply(world, calc::Multiply(view, projection)); matrix_->World = world; matrix_->WorldInverseTranspose = calc::Transpose(calc::Inverse(world));
}

void Application::Draw() {
	dx_.BeginFrame();

	ID3D12GraphicsCommandList* commandList = dx_.CommandList();

	commandList->SetGraphicsRootSignature(rootSignature_.Get());

	pipeline_.Bind(commandList);

	ID3D12DescriptorHeap* descriptorHeaps[] = {dx_.SrvHeap()};

	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->IASetVertexBuffers(0, 1, &vertexView_);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	commandList->SetGraphicsRootConstantBufferView(1, matrixResource_->GetGPUVirtualAddress());

	// fence.mtlから読み込んだテクスチャ
	commandList->SetGraphicsRootDescriptorTable(2, dx_.GpuSrv(2));

	commandList->SetGraphicsRootConstantBufferView(3, lightResource_->GetGPUVirtualAddress());

	commandList->DrawInstanced(static_cast<UINT>(model_.vertices.size()), 1, 0, 0);

#ifdef USE_IMGUI
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif

	dx_.EndFrame();
}

void Application::Finalize() {
	dx_.Finalize();
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
#endif
	input_.Finalize(); audio_.Finalize(); window_.Finalize(); CoUninitialize();
}
