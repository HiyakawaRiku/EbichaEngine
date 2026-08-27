#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
#include <vector>
#include <string>
#include <wrl.h>

class DebugRenderer {
public:
    struct Vertex {
        Vector4 pos;
        Vector4 color;
    };

    struct LineRequest {
        Vector3 start;
        Vector3 end;
        Vector4 color;
        bool depthEnable;  // 深度テストを行うか (falseで壁の裏側も透視)
        float duration;    // 表示維持時間 (0なら1フレームのみ)
    };

public:
    static void Initialize();
    static void Finalize();
    static void Update(float deltaTime); // Duration管理用更新関数

    // --- 基本描画 API ---
    static void AddLine(const Vector3& start, const Vector3& end, const Vector4& color, bool depthEnable = true, float duration = 0.0f);
    static void AddGrid(float size, int divisions, const Vector4& color);
    static void AddWireSphere(const Vector3& center, float radius, int tessellation, const Vector4& color, bool depthEnable = true, float duration = 0.0f);

    // --- 拡張プリミティブ API ---
    // 座標軸描画 (X:赤, Y:緑, Z:青)
    static void AddAxis(const Vector3& position, float length = 1.0f, bool depthEnable = true, float duration = 0.0f);
    // AABB (軸平行バウンディングボックス)
    static void AddWireAABB(const Vector3& min, const Vector3& max, const Vector4& color, bool depthEnable = true, float duration = 0.0f);
    // OBB (有向バウンディングボックス)
    static void AddWireOBB(const Vector3& center, const Vector3& halfExtents, const Matrix4x4& rotate, const Vector4& color, bool depthEnable = true, float duration = 0.0f);
    static void AddWireOBB(const Matrix4x4& transform, const Vector3& halfExtents, const Vector4& color, bool depthEnable = true, float duration = 0.0f);

    // 描画実行
    static void Flush(Camera* camera);

private:
    static std::vector<LineRequest> m_requests;

    static Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    static Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateDepth;   // 深度有効PSO
    static Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateNoDepth; // 深度無効(X-Ray)PSO

    static Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    static Microsoft::WRL::ComPtr<ID3D12Resource> m_constBuffer;

    static const UINT m_maxVertices = 8000; // 形状追加に伴い上限を拡張
};