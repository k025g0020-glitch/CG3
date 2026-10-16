#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "calc.h"

struct VertexData { Vector4 position; Vector2 texcoord; Vector3 normal; };
struct MaterialData { std::string textureFilePath; };
struct ModelData { std::vector<VertexData> vertices; MaterialData material; };
struct Material { Vector4 color; int32_t enableLighting; float padding[3]; Matrix4x4 uvTransform; };
struct DirectionalLight { Vector4 color; Vector3 direction; float intensity; };
struct TransformationMatrix { Matrix4x4 WVP; Matrix4x4 World; Matrix4x4 WorldInverseTranspose; };

enum class BlendMode : uint32_t { None, Normal, Add, Subtract, Multiply, Screen, Count };
