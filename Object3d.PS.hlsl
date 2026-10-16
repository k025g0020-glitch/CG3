#include "Object3d.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(
        float4(input.texcoord, 0.0f, 1.0f),
        gMaterial.uvTransform
    );

    float4 textureColor = gTexture.Sample(
        gSampler,
        transformedUV.xy
    );

    // 透明部分は色もDepthも書き込まない
    // discard以降のPixel Shader処理も実行されない
    if (textureColor.a <= 0.5f)
    {
        discard;
    }

    // ライティングなしの基本色
    output.color = gMaterial.color * textureColor;

    if (gMaterial.enableLighting != 0)
    {
        float3 normal = normalize(input.normal);

        float NdotL = dot(
            normal,
            -gDirectionalLight.direction
        );

        // Half Lambert
        float halfLambert =
            pow(NdotL * 0.5f + 0.5f, 2.0f);

        output.color.rgb =
            gMaterial.color.rgb *
            textureColor.rgb *
            gDirectionalLight.color.rgb *
            halfLambert *
            gDirectionalLight.intensity;

        output.color.a =
            gMaterial.color.a *
            textureColor.a;
    }

    return output;
}