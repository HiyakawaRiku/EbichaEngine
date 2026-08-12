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

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius; // 影響範囲（距離による減衰用）
    float decay; // 減衰率
    float padding[2];
};

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLight> gPointLight : register(b3);
ConstantBuffer<SpotLight> gSpotLight : register(b4);

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

    // 共通ベクトルの計算
    float3 N = normalize(input.normal);
    float3 V = normalize(gCamera.worldPosition - input.worldPosition); // 視線に向かうベクトル

    float3 totalDiffuse = float3(0.0f, 0.0f, 0.0f);
    float3 totalSpecular = float3(0.0f, 0.0f, 0.0f);

    // =========================================================
    // 1. 平行光源 (Directional Light)
    // =========================================================
    {
        float3 L = normalize(-gDirectionalLight.direction);
        float NdotL = dot(N, L);
        float lightFactor = 0.0f;

        if (gMaterial.lightingType == 1) // Lambert
        {
            lightFactor = saturate(NdotL);
        }
        else if (gMaterial.lightingType == 2) // Half-Lambert
        {
            float halfLambert = NdotL * 0.5f + 0.5f;
            lightFactor = halfLambert * halfLambert;
        }

        float3 H = normalize(L + V);
        float NdotH = saturate(dot(N, H));
        float specularPow = (NdotL > 0.0f) ? pow(NdotH, gMaterial.shininess) : 0.0f;

        float3 lightColor = gDirectionalLight.color.rgb * gDirectionalLight.intensity;
        totalDiffuse += gMaterial.color.rgb * textureColor.rgb * lightColor * lightFactor;
        totalSpecular += lightColor * specularPow;
    }

    // =========================================================
    // 2. ポイントライト (Point Light)
    // =========================================================
    {
        float3 L = normalize(gPointLight.position - input.worldPosition); // ライトへ向かうベクトル
        float distance = length(gPointLight.position - input.worldPosition); // ライトとの距離

        // 資料に基づく距離減衰の計算
        float pointFactor = pow(saturate(-distance / gPointLight.radius + 1.0f), gPointLight.decay);

        float NdotL = dot(N, L);
        float lightFactor = 0.0f;

        if (gMaterial.lightingType == 1) // Lambert
        {
            lightFactor = saturate(NdotL);
        }
        else if (gMaterial.lightingType == 2) // Half-Lambert
        {
            float halfLambert = NdotL * 0.5f + 0.5f;
            lightFactor = halfLambert * halfLambert;
        }

        float3 H = normalize(L + V);
        float NdotH = saturate(dot(N, H));
        float specularPow = (NdotL > 0.0f) ? pow(NdotH, gMaterial.shininess) : 0.0f;

        float3 lightColor = gPointLight.color.rgb * gPointLight.intensity * pointFactor;
        totalDiffuse += gMaterial.color.rgb * textureColor.rgb * lightColor * lightFactor;
        totalSpecular += lightColor * specularPow;
    }

    // =========================================================
    // 3. スポットライト (Spot Light)
    // =========================================================
    {
        float3 L = normalize(gSpotLight.position - input.worldPosition); // ライトへ向かうベクトル
        float distance = length(gSpotLight.position - input.worldPosition); // ライトとの距離

        // 距離による減衰 (PointLightと同様)
        float distanceFactor = pow(saturate(-distance / gSpotLight.distance + 1.0f), gSpotLight.decay);

        // 角度（Falloff）による減衰の計算
        // gSpotLight.direction は光の照射方向なので、表面からライトに向かうベクトル -L との余弦を取る
        float cosAngle = dot(-L, normalize(gSpotLight.direction));
        float falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));

        // 最終的なスポットライトの減衰係数
        float spotFactor = distanceFactor * falloffFactor;

        float NdotL = dot(N, L);
        float lightFactor = 0.0f;

        if (gMaterial.lightingType == 1) // Lambert
        {
            lightFactor = saturate(NdotL);
        }
        else if (gMaterial.lightingType == 2) // Half-Lambert
        {
            float halfLambert = NdotL * 0.5f + 0.5f;
            lightFactor = halfLambert * halfLambert;
        }

        float3 H = normalize(L + V);
        float NdotH = saturate(dot(N, H));
        float specularPow = (NdotL > 0.0f) ? pow(NdotH, gMaterial.shininess) : 0.0f;

        float3 lightColor = gSpotLight.color.rgb * gSpotLight.intensity * spotFactor;
        totalDiffuse += gMaterial.color.rgb * textureColor.rgb * lightColor * lightFactor;
        totalSpecular += lightColor * specularPow;
    }

    // 最終カラーの加算出力
    output.color.rgb = totalDiffuse + totalSpecular;
    output.color.a = gMaterial.color.a * textureColor.a;

    return output;
}