#pragma once
#include <d3d12.h>

enum class BlendMode {
    kNone,      // ブレンドなし
    kNormal,    // 通常αブレンド
    kAdd,       // 加算
    kSubtract,  // 減算
    kMultiply,  // 乗算
    kScreen,    // スクリーン
    kCount,     // 個数（配列サイズ用）
};

D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode);
