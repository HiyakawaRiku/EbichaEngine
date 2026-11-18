#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "BlendMode.h"

class PipelineSet {
public:
    void Create(ID3D12Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);
    void Reset();

    // 指定モードのPSOをセットする（前回と同じなら何もしない）
    void Apply(ID3D12GraphicsCommandList* commandList, BlendMode mode);

    // 「まだ何もセットしていない」状態に戻す（描き始めに呼ぶ）
    void ResetCurrent() { currentMode_ = BlendMode::kCount; }

private:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> psos_[(int)BlendMode::kCount];

    // 最後にセットしたモード（kCount＝まだ何もセットしていない）
    BlendMode currentMode_ = BlendMode::kCount;
};