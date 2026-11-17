#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "BlendMode.h"

// 1つのPSO設定から、ブレンドモードの数だけPSOを作って持っておくクラス
class PipelineSet {
public:
    // desc：ブレンド以外の設定（シェーダーなど）を済ませたもの
    void Create(ID3D12Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);

    // 指定したモードのPSOを返す
    ID3D12PipelineState* Get(BlendMode mode) const;

    // 全部のPSOを解放する
    void Reset();

private:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> psos_[(int)BlendMode::kCount];
};