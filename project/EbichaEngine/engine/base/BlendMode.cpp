#include "BlendMode.h"

D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode) {
    D3D12_BLEND_DESC desc{};
    auto& rt = desc.RenderTarget[0];
    rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    if (mode == BlendMode::kNone) { return desc; } // BlendEnable = FALSE

    rt.BlendEnable = TRUE;
    rt.SrcBlendAlpha = D3D12_BLEND_ONE;
    rt.DestBlendAlpha = D3D12_BLEND_ZERO;
    rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;

    switch (mode) {
    case BlendMode::kNormal:   rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;     rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; rt.BlendOp = D3D12_BLEND_OP_ADD;          break;
    case BlendMode::kAdd:      rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;     rt.DestBlend = D3D12_BLEND_ONE;           rt.BlendOp = D3D12_BLEND_OP_ADD;          break;
    case BlendMode::kSubtract: rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;     rt.DestBlend = D3D12_BLEND_ONE;           rt.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; break;
    case BlendMode::kMultiply: rt.SrcBlend = D3D12_BLEND_ZERO;          rt.DestBlend = D3D12_BLEND_SRC_COLOR;     rt.BlendOp = D3D12_BLEND_OP_ADD;          break;
    case BlendMode::kScreen:   rt.SrcBlend = D3D12_BLEND_INV_DEST_COLOR; rt.DestBlend = D3D12_BLEND_ONE;          rt.BlendOp = D3D12_BLEND_OP_ADD;          break;
    default: break;
    }
    return desc;
}
