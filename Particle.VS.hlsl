#include "Particle.hlsli"

// ConstantBufferからStructuredBufferへ変更（SRVなのでregisterはt）
StructuredBuffer<TransformationMatrix> gTransformationMatrices : register(t0);

struct VertexShaderInput{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

// SV_InstanceID : 0 <= instanceId <= (instanceCount - 1)
VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID){
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrices[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrices[instanceId].World));
    return output;
}
