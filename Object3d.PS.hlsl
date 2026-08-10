#include "object3d.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float4 color;
    int lightingType;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    // UV変換とテクスチャサンプリング
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    // Alpha Cutoff (Alpha Test)
    if (textureColor.a <= 0.5f)
    {
        discard;
    }

    PixelShaderOutput output;

    // 0: ライティングなし (Unlit)
    if (gMaterial.lightingType == 0)
    {
        output.color = textureColor * gMaterial.color;
        return output;
    }

    // ベクトルの正規化
    float3 N = normalize(input.normal);
    float3 L = normalize(-gDirectionalLight.direction); // 光源に向かうベクトル
    float3 V = normalize(gCamera.worldPosition - input.worldPosition); // 視線に向かうベクトル

    // 拡散反射係数 (NdotL)
    float NdotL = dot(N, L);
    float lightFactor = 0.0f;

    if (gMaterial.lightingType == 1)
    {
        // 1: ランバート (Lambert)
        lightFactor = saturate(NdotL);
    }
    else if (gMaterial.lightingType == 2)
    {
        // 2: ハーフランバート (Half-Lambert)
        float halfLambert = NdotL * 0.5f + 0.5f;
        lightFactor = halfLambert * halfLambert;
    }

    // --- 鏡面反射 (Specular) の計算 (Blinn-Phong) ---
    float3 H = normalize(L + V); // ハーフベクトル
    float NdotH = saturate(dot(N, H));
    
    // NdotL > 0 (光が当たる表面) の場合のみハイライトを出す
    float specularPow = (NdotL > 0.0f) ? pow(NdotH, gMaterial.shininess) : 0.0f;

    // ライティングの結合
    float3 lightColor = gDirectionalLight.color.rgb * gDirectionalLight.intensity;
    float3 diffuse = gMaterial.color.rgb * textureColor.rgb * lightColor * lightFactor;
    float3 specular = lightColor * specularPow;

    output.color.rgb = diffuse + specular;
    output.color.a = gMaterial.color.a * textureColor.a;

    return output;
}