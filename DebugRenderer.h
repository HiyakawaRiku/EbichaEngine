#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
#include "Matrix.h" // ★ Vector3, Vector4 を使うために追加
#include <vector>
#include <string>
#include <wrl.h>

class DebugRenderer {
public:
    // DirectX::XMFLOAT4 から Vector4 に統一
    struct Vertex {
        Vector4 pos;
        Vector4 color;
    };

    // DirectX::XMFLOAT3/4 から Vector3/4 に統一
    struct LineRequest {
        Vector3 start;
        Vector3 end;
        Vector4 color;
    };

public:
    // 初期化と終了処理
    static void Initialize();
    static void Finalize();

    // ★ 引数の型を Vector3, Vector4 に修正
    static void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);
    static void AddGrid(float size, int divisions, const Vector4& color);
    static void AddWireSphere(const Vector3& center, float radius, int tessellation, const Vector4& color);

    // 既存のCameraクラスを受け取って一括描画する
    static void Flush(Camera* camera);

private:
    static std::vector<LineRequest> m_requests;

    static Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    static Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

    static Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    static Microsoft::WRL::ComPtr<ID3D12Resource> m_constBuffer;

    static const UINT m_maxVertices = 4000;
};