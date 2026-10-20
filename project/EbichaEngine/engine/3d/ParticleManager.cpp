#include "ParticleManager.h"
#include "TextureManager.h"
#include "Logger.h"
#include "Camera.h"
#include <cassert>

ParticleManager* ParticleManager::GetInstance()
{
    static ParticleManager instance;
    return &instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
    this->dxCommon_ = dxCommon;
    this->srvManager_ = srvManager;

    std::random_device seedGenerator;
    randomEngine.seed(seedGenerator());

    // Material リソース作成 (b0 用)[cite: 10]
    materialResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->enableLighting = 0;
    materialData->uvTransform = MakeIdentity4x4();

    // PSOと板ポリゴンの作成
    CreatePipelineState();
    CreateVertexResource();
}

void ParticleManager::Finalize()
{
    if (signatureBlob) signatureBlob->Release();
    if (errorBlob) errorBlob->Release();
    if (pixelShaderBlob) pixelShaderBlob->Release();
    if (vertexShaderBlob) vertexShaderBlob->Release();
}

void ParticleManager::Update()
{
    const float kDeltaTime = 1.0f / 60.0f;

    // カメラが存在する場合、ビルボード用の行列を計算
    Matrix4x4 backToFrontMatrix = MakeIdentity4x4();
    Matrix4x4 billboardMatrix = MakeIdentity4x4();

    if (camera_) {
        // ビュー行列の取得
        Matrix4x4 viewMatrix = camera_->GetViewMatrix();

        // ビュー行列の逆行列（カメラのワールド行列）から回転成分を取り出す
        // ビュー行列の平行移動部分をクリアして逆行列を取ることで、ビルボード行列を作成
        backToFrontMatrix = viewMatrix;
        backToFrontMatrix.m[3][0] = 0.0f;
        backToFrontMatrix.m[3][1] = 0.0f;
        backToFrontMatrix.m[3][2] = 0.0f;

        // ビュー行列の回転成分の逆行列（転置）がビルボード行列となる
        billboardMatrix = Inverse(backToFrontMatrix);
    }

    for (auto& [name, group] : particleGroups) {
        uint32_t instanceIndex = 0;

        for (auto it = group.particles.begin(); it != group.particles.end(); ) {
            it->currentTime += kDeltaTime;

            if (it->currentTime >= it->lifeTime) {
                it = group.particles.erase(it);
                continue;
            }

            it->transform.translate += it->velocity * kDeltaTime;

            float alpha = 1.0f - (it->currentTime / it->lifeTime);
            it->color.w = alpha;

            // 各変換行列を作成
            Matrix4x4 scaleMatrix = MakeScaleMatrix(it->transform.scale);
            Matrix4x4 rotateMatrix = MakeRotateXYZMatrix(it->transform.rotate);
            Matrix4x4 translateMatrix = MakeTranslateMatrix(it->transform.translate);

            // ビルボード行列を適用（Scale -> Rotate -> Billboard -> Translate）
            Matrix4x4 worldMatrix = Multiply(scaleMatrix, rotateMatrix);
            worldMatrix = Multiply(worldMatrix, billboardMatrix);
            worldMatrix = Multiply(worldMatrix, translateMatrix);

            Matrix4x4 wvpMatrix = worldMatrix;
            if (camera_) {
                wvpMatrix = Multiply(worldMatrix, camera_->GetViewProjectionMatrix());
            }

            group.instancingData[instanceIndex].WVP = wvpMatrix;
            group.instancingData[instanceIndex].World = worldMatrix;

            ++instanceIndex;
            ++it;
        }
    }
}

float ParticleManager::RandomFloat(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(randomEngine);
}

void ParticleManager::CreatePipelineState()
{
    HRESULT hr;

    // --- 1. RootSignature作成 ---
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // Descriptor Ranges
    // t0 (VS用): StructuredBuffer<TransformationMatrix>[cite: 11]
    D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
    descriptorRangeForInstancing[0].BaseShaderRegister = 0;
    descriptorRangeForInstancing[0].NumDescriptors = 1;
    descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // t0 (PS用): Texture2D<float4>[cite: 10]
    D3D12_DESCRIPTOR_RANGE descriptorRangeForTexture[1] = {};
    descriptorRangeForTexture[0].BaseShaderRegister = 0;
    descriptorRangeForTexture[0].NumDescriptors = 1;
    descriptorRangeForTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRangeForTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // RootParameters (計3つ)
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // Parameter 0: b0 (PS) -> ConstantBuffer<Material>[cite: 10]
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // Parameter 1: t0 (VS) -> StructuredBuffer (DescriptorTable)[cite: 11]
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // Parameter 2: t0 (PS) -> Texture2D (DescriptorTable)[cite: 10]
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeForTexture;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    // Static Sampler: s0 (PS)[cite: 10]
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    // InputLayout[cite: 11]
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    // BlendState
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    // RasterizerState
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    // Shader Compile
    vertexShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    pixelShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    // DepthStencilState
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // PSO生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.BlendState = blendDesc;
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));
}

void ParticleManager::CreateVertexResource()
{
    VertexData vertices[6] = {
        { { -0.5f, -0.5f, 0.0f, 1.0f },{ 0.0f, 1.0f },{ 0.0f, 0.0f, -1.0f } },
        { { -0.5f,  0.5f, 0.0f, 1.0f },{ 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f } },
        { { 0.5f, -0.5f, 0.0f, 1.0f },{ 1.0f, 1.0f },{ 0.0f, 0.0f, -1.0f } },
        { { -0.5f,  0.5f, 0.0f, 1.0f },{ 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f } },
        { { 0.5f,  0.5f, 0.0f, 1.0f },{ 1.0f, 0.0f },{ 0.0f, 0.0f, -1.0f } },
        { { 0.5f, -0.5f, 0.0f, 1.0f },{ 1.0f, 1.0f },{ 0.0f, 0.0f, -1.0f } },
    };

    vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(vertices));

    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, vertices, sizeof(vertices));

    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(vertices);
    vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void ParticleManager::Draw()
{
    // ヒープの事前準備[cite: 1]
    srvManager_->PreDraw();

    auto commandList = dxCommon_->GetCommandList();

    // 1. パイプライン設定
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(graphicsPipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // 2. RootParameter 0: Material 用 ConstantBuffer (b0)[cite: 10]
    commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

    // 3. グループごとに描画
    for (const auto& [name, group] : particleGroups) {
        if (group.particles.empty()) {
            continue;
        }

        // RootParameter 1: インスタンシングバッファ (VS用 t0)[cite: 11]
        srvManager_->SetGraphicsRootDescriptorTable(1, group.instancingSrvIndex);

        // RootParameter 2: テクスチャ (PS用 t0)[cite: 10]
        srvManager_->SetGraphicsRootDescriptorTable(2, group.srvIndex);

        uint32_t instanceCount = static_cast<uint32_t>(group.particles.size());
        commandList->DrawInstanced(6, instanceCount, 0, 0);
    }
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath)
{
    if (particleGroups.find(name) != particleGroups.end()) {
        return;
    }

    ParticleGroup group;
    group.textureFilePath = textureFilePath;

    group.srvIndex = TextureManager::GetInstance()->Load(textureFilePath);

    uint32_t bufferSize = sizeof(ParticleForGPU) * ParticleGroup::kMaxInstance;
    group.instancingResource = CreateBufferResource(dxCommon_->GetDevice(), bufferSize);
    group.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&group.instancingData));

    group.instancingSrvIndex = srvManager_->Allocate();
    srvManager_->CreateSRVforStructuredBuffer(
        group.instancingSrvIndex,
        group.instancingResource.Get(),
        ParticleGroup::kMaxInstance,
        sizeof(ParticleForGPU)
    );

    particleGroups[name] = group;
}

void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count)
{
    auto it = particleGroups.find(name);
    if (it == particleGroups.end()) return;

    ParticleGroup& group = it->second;

    for (uint32_t i = 0; i < count; ++i) {
        Particle p{};
        p.transform.translate = position;
        p.transform.scale = { 1.0f, 1.0f, 1.0f };
        p.transform.rotate = { 0.0f, 0.0f, 0.0f };

        p.velocity = {
            RandomFloat(-1.0f, 1.0f),
            RandomFloat(1.0f, 3.0f),
            RandomFloat(-1.0f, 1.0f)
        };
        p.lifeTime = RandomFloat(1.0f, 3.0f);
        p.color = { 1.0f, 1.0f, 1.0f, 1.0f };

        group.particles.push_back(p);
    }
}