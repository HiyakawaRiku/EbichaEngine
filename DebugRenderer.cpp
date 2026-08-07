#include "DebugRenderer.h"
#include <cassert>

// 静的メンバ変数の実体定義
std::vector<DebugRenderer::LineRequest> DebugRenderer::m_requests;
Microsoft::WRL::ComPtr<ID3D12RootSignature> DebugRenderer::m_rootSignature;
Microsoft::WRL::ComPtr<ID3D12PipelineState> DebugRenderer::m_pipelineState;
Microsoft::WRL::ComPtr<ID3D12Resource> DebugRenderer::m_vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW DebugRenderer::m_vertexBufferView{};
Microsoft::WRL::ComPtr<ID3D12Resource> DebugRenderer::m_constBuffer;

void DebugRenderer::Initialize() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    assert(device != nullptr);

    m_requests.reserve(m_maxVertices / 2);

    // 1. ルートシグネチャの作成 (b0レジスタにViewProjection行列をセットする用)
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

    // 2. インプットレイアウト (POSITION と COLOR)
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "COLOR";
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{ inputElementDescs, _countof(inputElementDescs) };

    // 3. シェーダーのコンパイル (DirectXCommon.h のグローバル関数を使用)
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
    dxcUtils->CreateDefaultIncludeHandler(&includeHandler);

    // グローバル定義されている CompileShader を直接呼び出し
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = DirectXUtils::CompileShader(L"DebugLine.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob = DirectXUtils::CompileShader(L"DebugLine.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

    // 4. パイプライン状態 (PSO) の作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.InputLayout = inputLayoutDesc;
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // デバッグ線なのでカリングなし
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.AntialiasedLineEnable = TRUE;

    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // DirectXCommon.cpp のブレンド指定と同期
    psoDesc.PrimitiveTopologyType =D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; // 線用

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    psoDesc.DepthStencilState.DepthEnable = true;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    assert(SUCCEEDED(hr));

    // 5. バッファ作成 (DirectXCommon.h のグローバル関数を使用)
    m_vertexBuffer= DirectXUtils::CreateBufferResource(device, sizeof(Vertex) * m_maxVertices);
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(Vertex) * m_maxVertices;
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);

    // 行列用の定数バッファ (256バイトアライメント)
    m_constBuffer= DirectXUtils::CreateBufferResource(device, (sizeof(Matrix4x4) + 255) & ~255);
}

void DebugRenderer::Finalize() {
    m_requests.clear();
    m_rootSignature.Reset();
    m_pipelineState.Reset();
    m_vertexBuffer.Reset();
    m_constBuffer.Reset();
}

// ★ 引数を Vector3, Vector4 に修正
void DebugRenderer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
    if (m_requests.size() >= m_maxVertices / 2) return;
    m_requests.push_back({ start, end, color });
}

// ★ 引数を Vector4 に修正
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

// ★ 引数を Vector3, Vector4 に修正
void DebugRenderer::AddWireSphere(const Vector3& center, float radius, int tessellation, const Vector4& color) {
    for (int i = 0; i < tessellation; ++i) {
        float phi1 = DirectX::XM_2PI * (float)i / tessellation;
        float phi2 = DirectX::XM_2PI * (float)(i + 1) / tessellation;
        for (int j = 0; j < tessellation; ++j) {
            float theta1 = DirectX::XM_PI * (float)j / tessellation;
            float theta2 = DirectX::XM_PI * (float)(j + 1) / tessellation;

            Vector3 p1 = {
                center.x + radius * sinf(theta1) * cosf(phi1),
                center.y + radius * cosf(theta1),
                center.z + radius * sinf(theta1) * sinf(phi1)
            };
            Vector3 p2 = {
                center.x + radius * sinf(theta2) * cosf(phi1),
                center.y + radius * cosf(theta2),
                center.z + radius * sinf(theta2) * sinf(phi1)
            };
            AddLine(p1, p2, color);

            Vector3 p3 = {
                center.x + radius * sinf(theta1) * cosf(phi2),
                center.y + radius * cosf(theta1),
                center.z + radius * sinf(theta1) * sinf(phi2)
            };
            AddLine(p1, p3, color);
        }
    }
}

void DebugRenderer::Flush(Camera* camera) {
    if (m_vertexBuffer == nullptr) {
        m_requests.clear();
        return;
    }
    if (m_requests.empty()) return;

    auto commandList = DirectXCommon::GetInstance()->GetCommandList();

    // ビュー・プロジェクション行列の計算
    Matrix4x4 viewMatrix = camera->GetViewMatrix();
    Matrix4x4 projectionMatrix = camera->GetProjectionMatrix();

    Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

    // 1. 頂点バッファデータの作成
    std::vector<Vertex> vertices;
    vertices.reserve(m_requests.size() * 2);
    for (const auto& req : m_requests) {
        // ★ Vector4 (x,y,z,w) の形に合わせてすっきり投入
        vertices.push_back({ {req.start.x, req.start.y, req.start.z, 1.0f}, req.color });
        vertices.push_back({ {req.end.x, req.end.y, req.end.z, 1.0f}, req.color });
    }

    // 2. 転送
    void* pVertexData = nullptr;
    m_vertexBuffer->Map(0, nullptr, &pVertexData);
    memcpy(pVertexData, vertices.data(), sizeof(Vertex) * vertices.size());
    m_vertexBuffer->Unmap(0, nullptr);

    void* pConstData = nullptr;
    m_constBuffer->Map(0, nullptr, &pConstData);
    memcpy(pConstData, &viewProjectionMatrix, sizeof(Matrix4x4));
    m_constBuffer->Unmap(0, nullptr);

    // 3. 描画コマンドの積載
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    commandList->SetGraphicsRootConstantBufferView(0, m_constBuffer->GetGPUVirtualAddress());

    commandList->DrawInstanced((UINT)vertices.size(), 1, 0, 0);

    // 4. リクエストのリセット
    m_requests.clear();
}