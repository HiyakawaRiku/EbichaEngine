#include "DebugRenderer.h"
#include <cassert>
#include <algorithm>

std::vector<DebugRenderer::LineRequest> DebugRenderer::m_requests;
Microsoft::WRL::ComPtr<ID3D12RootSignature> DebugRenderer::m_rootSignature;
Microsoft::WRL::ComPtr<ID3D12PipelineState> DebugRenderer::m_pipelineStateDepth;
Microsoft::WRL::ComPtr<ID3D12PipelineState> DebugRenderer::m_pipelineStateNoDepth;
Microsoft::WRL::ComPtr<ID3D12Resource> DebugRenderer::m_vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW DebugRenderer::m_vertexBufferView{};
Microsoft::WRL::ComPtr<ID3D12Resource> DebugRenderer::m_constBuffer;

void DebugRenderer::Initialize() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    assert(device != nullptr);

    m_requests.reserve(m_maxVertices / 2);

    // 1. ルートシグネチャの作成
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_ROOT_PARAMETER rootParam[1] = {};
    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParam[0].Descriptor.ShaderRegister = 0; // b0
    rootSigDesc.pParameters = rootParam;
    rootSigDesc.NumParameters = _countof(rootParam);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    assert(SUCCEEDED(hr));
    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    assert(SUCCEEDED(hr));

    // 2. インプットレイアウト
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "COLOR";
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{ inputElementDescs, _countof(inputElementDescs) };

    // 3. シェーダーのコンパイル
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
    dxcUtils->CreateDefaultIncludeHandler(&includeHandler);

    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = DirectXUtils::CompileShader(L"DebugLine.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob = DirectXUtils::CompileShader(L"DebugLine.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

    // 4. パイプライン状態 (PSO) 作成 - 深度有効用
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.InputLayout = inputLayoutDesc;
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.AntialiasedLineEnable = TRUE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    psoDesc.DepthStencilState.DepthEnable = true;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateDepth));
    assert(SUCCEEDED(hr));

    // 深度無効用 (X-Ray用PSO)
    psoDesc.DepthStencilState.DepthEnable = false;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateNoDepth));
    assert(SUCCEEDED(hr));

    // 5. バッファ作成
    m_vertexBuffer = DirectXUtils::CreateBufferResource(device, sizeof(Vertex) * m_maxVertices);
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(Vertex) * m_maxVertices;
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);

    m_constBuffer = DirectXUtils::CreateBufferResource(device, (sizeof(Matrix4x4) + 255) & ~255);
}

void DebugRenderer::Finalize() {
    m_requests.clear();
    m_rootSignature.Reset();
    m_pipelineStateDepth.Reset();
    m_pipelineStateNoDepth.Reset();
    m_vertexBuffer.Reset();
    m_constBuffer.Reset();
}

void DebugRenderer::Update(float deltaTime) {
    // duration が設定されているリクエストのタイマー更新＆破棄処理
    for (auto it = m_requests.begin(); it != m_requests.end(); ) {
        if (it->duration > 0.0f) {
            it->duration -= deltaTime;
            if (it->duration <= 0.0f) {
                it = m_requests.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void DebugRenderer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color, bool depthEnable, float duration) {
    if (m_requests.size() >= m_maxVertices / 2) return;
    m_requests.push_back({ start, end, color, depthEnable, duration });
}

void DebugRenderer::AddGrid(float size, int divisions, const Vector4& color) {
    float halfSize = size / 2.0f;
    float step = size / divisions;

    for (int i = 0; i <= divisions; ++i) {
        float x = -halfSize + i * step;
        AddLine({ x, 0.0f, -halfSize }, { x, 0.0f, halfSize }, color);

        float z = -halfSize + i * step;
        AddLine({ -halfSize, 0.0f, z }, { halfSize, 0.0f, z }, color);
    }
}

void DebugRenderer::AddWireSphere(const Vector3& center, float radius, int tessellation, const Vector4& color, bool depthEnable, float duration) {
    for (int i = 0; i < tessellation; ++i) {
        float phi1 = DirectX::XM_2PI * (float)i / tessellation;
        float phi2 = DirectX::XM_2PI * (float)(i + 1) / tessellation;
        for (int j = 0; j < tessellation; ++j) {
            float theta1 = DirectX::XM_PI * (float)j / tessellation;
            float theta2 = DirectX::XM_PI * (float)(j + 1) / tessellation;

            Vector3 p1 = { center.x + radius * sinf(theta1) * cosf(phi1), center.y + radius * cosf(theta1), center.z + radius * sinf(theta1) * sinf(phi1) };
            Vector3 p2 = { center.x + radius * sinf(theta2) * cosf(phi1), center.y + radius * cosf(theta2), center.z + radius * sinf(theta2) * sinf(phi1) };
            AddLine(p1, p2, color, depthEnable, duration);

            Vector3 p3 = { center.x + radius * sinf(theta1) * cosf(phi2), center.y + radius * cosf(theta1), center.z + radius * sinf(theta1) * sinf(phi2) };
            AddLine(p1, p3, color, depthEnable, duration);
        }
    }
}

void DebugRenderer::AddAxis(const Vector3& position, float length, bool depthEnable, float duration) {
    AddLine(position, { position.x + length, position.y, position.z }, { 1.0f, 0.0f, 0.0f, 1.0f }, depthEnable, duration); // X (赤)
    AddLine(position, { position.x, position.y + length, position.z }, { 0.0f, 1.0f, 0.0f, 1.0f }, depthEnable, duration); // Y (緑)
    AddLine(position, { position.x, position.y, position.z + length }, { 0.0f, 0.0f, 1.0f, 1.0f }, depthEnable, duration); // Z (青)
}

void DebugRenderer::AddWireAABB(const Vector3& min, const Vector3& max, const Vector4& color, bool depthEnable, float duration) {
    Vector3 p[8] = {
        { min.x, min.y, min.z }, { max.x, min.y, min.z }, { max.x, max.y, min.z }, { min.x, max.y, min.z },
        { min.x, min.y, max.z }, { max.x, min.y, max.z }, { max.x, max.y, max.z }, { min.x, max.y, max.z },
    };

    // 底面
    AddLine(p[0], p[1], color, depthEnable, duration); AddLine(p[1], p[2], color, depthEnable, duration);
    AddLine(p[2], p[3], color, depthEnable, duration); AddLine(p[3], p[0], color, depthEnable, duration);
    // 天面
    AddLine(p[4], p[5], color, depthEnable, duration); AddLine(p[5], p[6], color, depthEnable, duration);
    AddLine(p[6], p[7], color, depthEnable, duration); AddLine(p[7], p[4], color, depthEnable, duration);
    // 柱
    AddLine(p[0], p[4], color, depthEnable, duration); AddLine(p[1], p[5], color, depthEnable, duration);
    AddLine(p[2], p[6], color, depthEnable, duration); AddLine(p[3], p[7], color, depthEnable, duration);
}

void DebugRenderer::AddWireOBB(const Vector3& center, const Vector3& halfExtents, const Matrix4x4& rotate, const Vector4& color, bool depthEnable, float duration) {
    Vector3 localPoints[8] = {
        { -halfExtents.x, -halfExtents.y, -halfExtents.z }, {  halfExtents.x, -halfExtents.y, -halfExtents.z },
        {  halfExtents.x,  halfExtents.y, -halfExtents.z }, { -halfExtents.x,  halfExtents.y, -halfExtents.z },
        { -halfExtents.x, -halfExtents.y,  halfExtents.z }, {  halfExtents.x, -halfExtents.y,  halfExtents.z },
        {  halfExtents.x,  halfExtents.y,  halfExtents.z }, { -halfExtents.x,  halfExtents.y,  halfExtents.z }
    };

    // ★ rotate 行列の平行移動成分(m[3][0]~[3][2])を無視して純粋な回転だけを適用する
    Matrix4x4 pureRotate = rotate;
    pureRotate.m[3][0] = 0.0f;
    pureRotate.m[3][1] = 0.0f;
    pureRotate.m[3][2] = 0.0f;

    Vector3 worldPoints[8];
    for (int i = 0; i < 8; ++i) {
        // 回転のみを適用してから、明確に center を1回だけ加算
        worldPoints[i] = Transforms(localPoints[i], pureRotate) + center;
    }

    // 底面
    AddLine(worldPoints[0], worldPoints[1], color, depthEnable, duration); AddLine(worldPoints[1], worldPoints[2], color, depthEnable, duration);
    AddLine(worldPoints[2], worldPoints[3], color, depthEnable, duration); AddLine(worldPoints[3], worldPoints[0], color, depthEnable, duration);
    // 天面
    AddLine(worldPoints[4], worldPoints[5], color, depthEnable, duration); AddLine(worldPoints[5], worldPoints[6], color, depthEnable, duration);
    AddLine(worldPoints[6], worldPoints[7], color, depthEnable, duration); AddLine(worldPoints[7], worldPoints[4], color, depthEnable, duration);
    // 柱
    AddLine(worldPoints[0], worldPoints[4], color, depthEnable, duration); AddLine(worldPoints[1], worldPoints[5], color, depthEnable, duration);
    AddLine(worldPoints[2], worldPoints[6], color, depthEnable, duration); AddLine(worldPoints[3], worldPoints[7], color, depthEnable, duration);
}

// 2. ★追加: OBBのワールド変換行列 (obb.transform) を直接渡す場合
void DebugRenderer::AddWireOBB(const Matrix4x4& transform, const Vector3& halfExtents, const Vector4& color, bool depthEnable, float duration) {
    Vector3 localPoints[8] = {
        { -halfExtents.x, -halfExtents.y, -halfExtents.z }, {  halfExtents.x, -halfExtents.y, -halfExtents.z },
        {  halfExtents.x,  halfExtents.y, -halfExtents.z }, { -halfExtents.x,  halfExtents.y, -halfExtents.z },
        { -halfExtents.x, -halfExtents.y,  halfExtents.z }, {  halfExtents.x, -halfExtents.y,  halfExtents.z },
        {  halfExtents.x,  halfExtents.y,  halfExtents.z }, { -halfExtents.x,  halfExtents.y,  halfExtents.z }
    };

    Vector3 worldPoints[8];
    for (int i = 0; i < 8; ++i) {
        // ワールド行列をそのまま使ってローカル座標をワールド座標へ変換
        worldPoints[i] = Transforms(localPoints[i], transform);
    }

    // 底面
    AddLine(worldPoints[0], worldPoints[1], color, depthEnable, duration); AddLine(worldPoints[1], worldPoints[2], color, depthEnable, duration);
    AddLine(worldPoints[2], worldPoints[3], color, depthEnable, duration); AddLine(worldPoints[3], worldPoints[0], color, depthEnable, duration);
    // 天面
    AddLine(worldPoints[4], worldPoints[5], color, depthEnable, duration); AddLine(worldPoints[5], worldPoints[6], color, depthEnable, duration);
    AddLine(worldPoints[6], worldPoints[7], color, depthEnable, duration); AddLine(worldPoints[7], worldPoints[4], color, depthEnable, duration);
    // 柱
    AddLine(worldPoints[0], worldPoints[4], color, depthEnable, duration); AddLine(worldPoints[1], worldPoints[5], color, depthEnable, duration);
    AddLine(worldPoints[2], worldPoints[6], color, depthEnable, duration); AddLine(worldPoints[3], worldPoints[7], color, depthEnable, duration);
}

void DebugRenderer::Flush(Camera* camera) {
    if (m_vertexBuffer == nullptr || m_requests.empty()) return;

    auto commandList = DirectXCommon::GetInstance()->GetCommandList();

    Matrix4x4 viewProjectionMatrix = Multiply(camera->GetViewMatrix(), camera->GetProjectionMatrix());

    // 定数バッファ転送
    void* pConstData = nullptr;
    m_constBuffer->Map(0, nullptr, &pConstData);
    memcpy(pConstData, &viewProjectionMatrix, sizeof(Matrix4x4));
    m_constBuffer->Unmap(0, nullptr);

    // 深度有効と深度無効(X-Ray)でグループ化して別々に描画
    std::vector<Vertex> verticesDepth;
    std::vector<Vertex> verticesNoDepth;

    for (const auto& req : m_requests) {
        Vertex v1 = { {req.start.x, req.start.y, req.start.z, 1.0f}, req.color };
        Vertex v2 = { {req.end.x, req.end.y, req.end.z, 1.0f}, req.color };

        if (req.depthEnable) {
            verticesDepth.push_back(v1);
            verticesDepth.push_back(v2);
        }
        else {
            verticesNoDepth.push_back(v1);
            verticesNoDepth.push_back(v2);
        }
    }

    // 全頂点データをまとめてバッファに転送
    void* pVertexData = nullptr;
    m_vertexBuffer->Map(0, nullptr, &pVertexData);

    size_t depthSize = sizeof(Vertex) * verticesDepth.size();
    size_t noDepthSize = sizeof(Vertex) * verticesNoDepth.size();

    if (depthSize > 0) memcpy(pVertexData, verticesDepth.data(), depthSize);
    if (noDepthSize > 0) memcpy((uint8_t*)pVertexData + depthSize, verticesNoDepth.data(), noDepthSize);

    m_vertexBuffer->Unmap(0, nullptr);

    // 共通の設定
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList->SetGraphicsRootConstantBufferView(0, m_constBuffer->GetGPUVirtualAddress());

    // 1. 深度有効線の描画
    if (!verticesDepth.empty()) {
        commandList->SetPipelineState(m_pipelineStateDepth.Get());
        commandList->DrawInstanced((UINT)verticesDepth.size(), 1, 0, 0);
    }

    // 2. 深度無効(X-Ray)線の描画
    if (!verticesNoDepth.empty()) {
        commandList->SetPipelineState(m_pipelineStateNoDepth.Get());
        commandList->DrawInstanced((UINT)verticesNoDepth.size(), 1, (UINT)verticesDepth.size(), 0);
    }

    // duration == 0.0f の単発描画リクエストのみ消去
    auto it = std::remove_if(m_requests.begin(), m_requests.end(), [](const LineRequest& req) {
        return req.duration <= 0.0f;
        });
    m_requests.erase(it, m_requests.end());
}