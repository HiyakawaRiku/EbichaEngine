#pragma once
#include <random>
#include "EMath.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <wrl.h>

class Camera;

class ParticleManager
{
public:
    // 頂点データ構造
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };

    // インスタンシングデータ (VS用: TransformationMatrix)[cite: 11]
    struct ParticleForGPU {
        Matrix4x4 WVP;
        Matrix4x4 World;
    };

    // HLSL側の Material 構造体 (b0 用) に合わせる[cite: 10]
    struct Material {
        Vector4 color;
        int32_t enableLighting;
        float padding[3]; // アライメント調整
        Matrix4x4 uvTransform;
    };

    // CPU側データ
    struct Particle {
        Transform transform;
        Vector3 velocity;
        Vector4 color;
        float lifeTime;
        float currentTime;
    };

    // グループ情報
    struct ParticleGroup {
        std::string textureFilePath;
        uint32_t srvIndex = 0;
        std::vector<Particle> particles;

        Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
        uint32_t instancingSrvIndex = 0;
        ParticleForGPU* instancingData = nullptr;

        static const uint32_t kMaxInstance = 1024;
    };

public:
    static ParticleManager* GetInstance();

    void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
    void Update();
    void Draw();
    void Finalize();

    void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);
    void Emit(const std::string& name, const Vector3& position, uint32_t count);
    float RandomFloat(float min, float max);

    void SetCamera(Camera* camera) { camera_ = camera; }

private:
    ParticleManager() = default;
    ~ParticleManager() = default;
    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;

    void CreatePipelineState();
    void CreateVertexResource();

private:
    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    Camera* camera_ = nullptr;

    // PSO & RootSignature
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;

    // 共通頂点リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    VertexData* vertexData = nullptr;

    // Material用 ConstantBuffer (b0)[cite: 10]
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
    Material* materialData = nullptr;

    std::unordered_map<std::string, ParticleGroup> particleGroups;
    std::mt19937 randomEngine;
};