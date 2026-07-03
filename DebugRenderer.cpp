#include "DebugRenderer.h"
#include <cassert>

// 静的メンバ変数の実体定義
std::vector<DebugRenderer::LineRequest> DebugRenderer::m_requests;
Microsoft::WRL::ComPtr<ID3D12RootSignature> DebugRenderer::m_rootSignature;
Microsoft::WRL::ComPtr<ID3D12PipelineState> DebugRenderer::m_pipelineState;
Microsoft::WRL::ComPtr<ID3D12Resource> DebugRenderer::m_vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW DebugRenderer::m_vertexBufferView{};
Microsoft::WRL::ComPtr<ID3D12Resource> DebugRenderer::m_constBuffer;

// main.cpp や Camera.cpp 周辺で使われている数理関数を外部参照
extern Matrix4x4 MakeAffineMatrix(DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotate, DirectX::XMFLOAT3 translate);
extern Matrix4x4 Inverse(const Matrix4x4& m);
extern Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ);
extern Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

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
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = ::CompileShader(L"DebugLine.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob = ::CompileShader(L"DebugLine.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

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
    m_vertexBuffer.Attach(::CreateBufferResource(device, sizeof(Vertex) * m_maxVertices));
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(Vertex) * m_maxVertices;
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);

    // 行列用の定数バッファ (256バイトアライメント)
    m_constBuffer.Attach(::CreateBufferResource(device, (sizeof(Matrix4x4) + 255) & ~255));
}

void DebugRenderer::Finalize() {
    m_requests.clear();
    m_vertexBuffer.Reset();
    m_constBuffer.Reset();
    m_pipelineState.Reset();
    m_rootSignature.Reset();
}

void DebugRenderer::AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color) {
    if (m_requests.size() * 2 >= m_maxVertices) return;
    m_requests.push_back({ start, end, color });
}

void DebugRenderer::AddGrid(float size, int divisions, const DirectX::XMFLOAT4& color) {
    float step = (size * 2.0f) / divisions;
    for (int i = 0; i <= divisions; ++i) {
        float pos = -size + i * step;
        AddLine({ -size, 0.0f, pos }, { size, 0.0f, pos }, color);
        AddLine({ pos, 0.0f, -size }, { pos, 0.0f, size }, color);
    }
}

void DebugRenderer::AddWireSphere(const DirectX::XMFLOAT3& center, float radius, int tessellation, const DirectX::XMFLOAT4& color) {
    for (int i = 0; i < tessellation; ++i) {
        float phi1 = DirectX::XM_2PI * (float)i / tessellation;
        float phi2 = DirectX::XM_2PI * (float)(i + 1) / tessellation;
        for (int j = 0; j < tessellation; ++j) {
            float theta1 = DirectX::XM_PI * (float)j / tessellation;
            float theta2 = DirectX::XM_PI * (float)(j + 1) / tessellation;

            DirectX::XMFLOAT3 p1 = {
                center.x + radius * sinf(theta1) * cosf(phi1),
                center.y + radius * cosf(theta1),
                center.z + radius * sinf(theta1) * sinf(phi1)
            };
            DirectX::XMFLOAT3 p2 = {
                center.x + radius * sinf(theta2) * cosf(phi1),
                center.y + radius * cosf(theta2),
                center.z + radius * sinf(theta2) * sinf(phi1)
            };
            AddLine(p1, p2, color);

            DirectX::XMFLOAT3 p3 = {
                center.x + radius * sinf(theta1) * cosf(phi2),
                center.y + radius * cosf(theta1),
                center.z + radius * sinf(theta1) * sinf(phi2)
            };
            AddLine(p1, p3, color);
        }
    }
}

void DebugRenderer::Flush(Camera* camera) {
    if (m_requests.empty() || !camera) return;

    ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
    assert(commandList != nullptr);

    // ★ 既存の Camera.cpp と全く同じ手順で View-Projection 行列を合成
    Matrix4x4 cameraMatrix = MakeAffineMatrix(camera->transform_.scale, camera->transform_.rotate, camera->transform_.translate);
    Matrix4x4 viewMatrix = Inverse(cameraMatrix);

    // WinAppのウィンドウサイズ定義を安全に引っ張るために camera オブジェクト経由等のアスペクト比計算に合わせる
    // (Camera.cppのロジックをそのまま流用)
    float windowWidth = 1280.0f;  // 必要に応じて適切なグローバル定義やwinApp_->kWindowWidthに変更してください
    float windowHeight = 720.0f;
    Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, windowWidth / windowHeight, 0.1f, 100.0f);

    // ワールド行列を掛け算する前の「ViewProjection行列」を作る
    Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

    // 1. 頂点バッファデータの作成
    std::vector<Vertex> vertices;
    vertices.reserve(m_requests.size() * 2);
    for (const auto& req : m_requests) {
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

    // 3. パイプライン・トポロジー設定
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->SetGraphicsRootConstantBufferView(0, m_constBuffer->GetGPUVirtualAddress());

    // 4. ドローコール
    commandList->DrawInstanced(static_cast<UINT>(vertices.size()), 1, 0, 0);

    m_requests.clear();
}