#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
#include <vector>
#include <string>
#include <wrl.h>
#include <directxmath.h>

class DebugRenderer {
public:
    struct Vertex {
        DirectX::XMFLOAT4 pos;
        DirectX::XMFLOAT4 color;
    };

    struct LineRequest {
        DirectX::XMFLOAT3 start;
        DirectX::XMFLOAT3 end;
        DirectX::XMFLOAT4 color;
    };

public:
    // 初期化と終了処理
    static void Initialize();
    static void Finalize();

    // 線の追加
    static void AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color);
    static void AddGrid(float size, int divisions, const DirectX::XMFLOAT4& color);
    static void AddWireSphere(const DirectX::XMFLOAT3& center, float radius, int tessellation, const DirectX::XMFLOAT4& color);

    // ★ 既存のCameraクラスを受け取って一括描画する
    static void Flush(Camera* camera);

private:
    static std::vector<LineRequest> m_requests;

    static Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    static Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    static Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    static Microsoft::WRL::ComPtr<ID3D12Resource> m_constBuffer; // ViewProjection行列用

    static const UINT m_maxVertices = 4000;
};