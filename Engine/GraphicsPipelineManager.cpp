#include "GraphicsPipelineManager.h"
#include "DirectXUtils.h"
#include <cassert>

void GraphicsPipelineManager::Initialize(ID3D12Device* device) {
    assert(device != nullptr);
    CreateRootSignature(device);
    CreateGraphicsPipelines(device);
}

void GraphicsPipelineManager::CreateRootSignature(ID3D12Device* device) {
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // インスタンスデータ用 SRV の Descriptor Range を作成
    D3D12_DESCRIPTOR_RANGE instanceSrvRange[1] = {};
    instanceSrvRange[0].BaseShaderRegister = 0; // register(t1)
    instanceSrvRange[0].NumDescriptors = 1;
    instanceSrvRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    instanceSrvRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[6] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0;

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 1;

    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[4].DescriptorTable.pDescriptorRanges = instanceSrvRange;
    rootParameters[4].DescriptorTable.NumDescriptorRanges = _countof(instanceSrvRange);

    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[5].Descriptor.ShaderRegister = 2;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    assert(SUCCEEDED(hr));

    hr = device->CreateRootSignature(0,
        signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

D3D12_BLEND_DESC GraphicsPipelineManager::CreateBlendDesc(BlendMode mode) const {
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    if (mode != BlendMode::kNone) {
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
    }

    switch (mode) {
    case BlendMode::kNone:
        break;
    case BlendMode::kNormal:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        break;
    case BlendMode::kAdd:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;
    case BlendMode::kSubtract:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;
    case BlendMode::kMultiply:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
        break;
    case BlendMode::kScreen:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;
    default:
        break;
    }

    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    return blendDesc;
}

void GraphicsPipelineManager::CreateGraphicsPipelines(ID3D12Device* device) {
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
    HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));

    // Object3D 用[cite: 5]
    Microsoft::WRL::ComPtr<IDxcBlob> objVSBlob = DirectXUtils::CompileShader(L"Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler); 
        Microsoft::WRL::ComPtr<IDxcBlob> objPSBlob = DirectXUtils::CompileShader(L"Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler); 

        // Particle 用
        Microsoft::WRL::ComPtr<IDxcBlob> particleVSBlob = DirectXUtils::CompileShader(L"Particle.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    Microsoft::WRL::ComPtr<IDxcBlob> particlePSBlob = DirectXUtils::CompileShader(L"Particle.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    if (depthMask) {
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    }
    else {
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    }
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    for (uint32_t b = 0; b < static_cast<uint32_t>(BlendMode::kCount); ++b) {
        BlendMode mode = static_cast<BlendMode>(b);
        graphicsPipelineStateDesc.BlendState = CreateBlendDesc(mode); 

        for (uint32_t dw = 0; dw < static_cast<uint32_t>(DepthWrite::kCount); ++dw) {
            DepthWrite depthWrite = static_cast<DepthWrite>(dw);

            // DepthStencilDesc の動的設定
            D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
            depthStencilDesc.DepthEnable = true;
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            depthStencilDesc.DepthWriteMask = (depthWrite == DepthWrite::kEnable)
                ? D3D12_DEPTH_WRITE_MASK_ALL
                : D3D12_DEPTH_WRITE_MASK_ZERO;

            graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;

            // --- Object3D PSO ---[cite: 5]
            graphicsPipelineStateDesc.VS = { objVSBlob->GetBufferPointer(), objVSBlob->GetBufferSize() };
            graphicsPipelineStateDesc.PS = { objPSBlob->GetBufferPointer(), objPSBlob->GetBufferSize() };
            device->CreateGraphicsPipelineState(
                &graphicsPipelineStateDesc,
                IID_PPV_ARGS(&graphicsPipelineStates_[static_cast<size_t>(PipelineType::kObject3D)][b][dw])
            );

            // --- Particle PSO ---[cite: 5]
            graphicsPipelineStateDesc.VS = { particleVSBlob->GetBufferPointer(), particleVSBlob->GetBufferSize() };
            graphicsPipelineStateDesc.PS = { particlePSBlob->GetBufferPointer(), particlePSBlob->GetBufferSize() };
            device->CreateGraphicsPipelineState(
                &graphicsPipelineStateDesc,
                IID_PPV_ARGS(&graphicsPipelineStates_[static_cast<size_t>(PipelineType::kParticle)][b][dw])
            );
        }
    }
}