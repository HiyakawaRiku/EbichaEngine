#include "Sprite.h"
#include "Object3d.h"
#include "Logger.h"

using namespace Logger;

void Sprite::Initialize(EDirectX*eDirectX)
{

	// Sprite用の頂点リソースを作る
	vertexResourceSprite = eDirectX->CreateBufferResource(sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	// 1枚目の三角形
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };


	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResourceSprite = eDirectX->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	// 今回は赤を書き込んでみる
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();


	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResourceSprite = eDirectX->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	// 単位行列を書きこんでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();


	// Sprite用の頂点リソースを作る
	indexResourceSprite = eDirectX->CreateBufferResource(sizeof(VertexData) * 6);

	// リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;


	//インデックスリソースにデータを書き込む
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;	indexDataSprite[1] = 1;	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;	indexDataSprite[4] = 3;	indexDataSprite[5] = 2;


	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC object3dDescriptionRootSignature{};
	object3dDescriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;  // 0から始まる
	descriptorRange[0].NumDescriptors = 1;  // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;  // Offsetを自動計算

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;  // 0から始まる
	descriptorRangeForInstancing[0].NumDescriptors = 1;  // 数は1つ
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  // SRVを使う
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;  // Offsetを自動計算

	// RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform
	D3D12_ROOT_PARAMETER object3dRootParameters[4] = {};

	object3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	object3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;   // PixelShaderで使う
	object3dRootParameters[0].Descriptor.ShaderRegister = 0;   // レジスタ番号0を使う
	object3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	object3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;  // VertexShaderで使う
	object3dRootParameters[1].Descriptor.ShaderRegister = 0;   // レジスタ番号0を使う
	object3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	object3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	object3dRootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	object3dRootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	object3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	object3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	object3dRootParameters[3].Descriptor.ShaderRegister = 1;   // レジスタ番号1を使う
	object3dDescriptionRootSignature.pParameters = object3dRootParameters;   // ルートパラメータ配列へのポインタ
	object3dDescriptionRootSignature.NumParameters = _countof(object3dRootParameters);   // 配列の長さ

	D3D12_STATIC_SAMPLER_DESC object3dStaticSamplers[1] = {};
	object3dStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	object3dStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	object3dStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	object3dStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	object3dStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	object3dStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	object3dStaticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	object3dStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	object3dDescriptionRootSignature.pStaticSamplers = object3dStaticSamplers;
	object3dDescriptionRootSignature.NumStaticSamplers = _countof(object3dStaticSamplers);

	// シリアライズしてバイナリにする

	HRESULT hr = D3D12SerializeRootSignature(&object3dDescriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &object3dSignatureBlob, &object3dErrorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(object3dErrorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = eDirectX->GetDevice()->CreateRootSignature(0,
		object3dSignatureBlob->GetBufferPointer(), object3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&object3dRootSignature));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC object3dInputElementDescs[3] = {};
	object3dInputElementDescs[0].SemanticName = "POSITION";
	object3dInputElementDescs[0].SemanticIndex = 0;
	object3dInputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	object3dInputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	object3dInputElementDescs[1].SemanticName = "TEXCOORD";
	object3dInputElementDescs[1].SemanticIndex = 0;
	object3dInputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	object3dInputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	object3dInputElementDescs[2].SemanticName = "NORMAL";
	object3dInputElementDescs[2].SemanticIndex = 0;
	object3dInputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	object3dInputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC object3dInputLayoutDesc{};
	object3dInputLayoutDesc.pInputElementDescs = object3dInputElementDescs;
	object3dInputLayoutDesc.NumElements = _countof(object3dInputElementDescs);

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc[6]{};

	blendDesc[0].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	blendDesc[1].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[1].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[1].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[1].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[1].RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc[1].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[1].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[1].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	blendDesc[2].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[2].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[2].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[2].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[2].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc[2].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[2].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[2].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	blendDesc[3].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[3].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[3].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[3].RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	blendDesc[3].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc[3].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[3].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[3].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	blendDesc[4].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[4].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[4].RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
	blendDesc[4].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[4].RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
	blendDesc[4].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[4].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[4].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	blendDesc[5].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[5].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[5].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[5].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[5].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc[5].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[5].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[5].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;


	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


	// Shaderをコンパイルする
	object3dVertexShaderBlob = eDirectX->CompileShader(L"Object3D.VS.hlsl",
		L"vs_6_0");
	assert(object3dVertexShaderBlob != nullptr);

	object3dPixelShaderBlob = eDirectX->CompileShader(L"Object3D.PS.hlsl", L"ps_6_0");
	assert(object3dPixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC object3dDepthStencilDesc{};
	// Depthの機能を有効化する
	object3dDepthStencilDesc.DepthEnable = true;
	// 書き込みします
	object3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	object3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC object3dPipelineStateDesc{};
	object3dPipelineStateDesc.pRootSignature = object3dRootSignature.Get(); // RootSignature
	object3dPipelineStateDesc.InputLayout = object3dInputLayoutDesc; // InputLayout
	object3dPipelineStateDesc.VS = { object3dVertexShaderBlob->GetBufferPointer(),
		object3dVertexShaderBlob->GetBufferSize() }; // VertexShader
	object3dPipelineStateDesc.PS = { object3dPixelShaderBlob->GetBufferPointer(),
		object3dPixelShaderBlob->GetBufferSize() }; // Pixel Shader
	object3dPipelineStateDesc.BlendState = blendDesc[1]; // BlendState
	object3dPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	object3dPipelineStateDesc.NumRenderTargets = 1;
	object3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	object3dPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定（気にしなくて良い）
	object3dPipelineStateDesc.SampleDesc.Count = 1;
	object3dPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	object3dPipelineStateDesc.DepthStencilState = object3dDepthStencilDesc;
	object3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	hr = eDirectX->GetDevice()->CreateGraphicsPipelineState(&object3dPipelineStateDesc,
		IID_PPV_ARGS(&object3dPipelineState));
	assert(SUCCEEDED(hr));

}

void Sprite::Update()
{

	// Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
	Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(EWindow::kClientWidth), float(EWindow::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
	transformationMatrixDataSprite->World = worldViewProjectionMatrixSprite;
	transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
	materialDataSprite->uvTransform = uvTransformMatrix;

}

void Sprite::Draw(EDirectX*eDirectX, D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU)
{

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	eDirectX->GetCommandList()->SetGraphicsRootSignature(object3dRootSignature.Get());
	eDirectX->GetCommandList()->SetPipelineState(object3dPipelineState.Get()); // PSOを設定
	// Spriteの描画。変更が必要なものだけ変更する
	eDirectX->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定
	eDirectX->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	eDirectX->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite); // IBVを設定
	// マテリアルCBufferの場所を設定
	eDirectX->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
	// TransformationMatrixCBufferの場所を設定
	eDirectX->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameters[2]である。
	eDirectX->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
	// 描画！（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い
	eDirectX->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::Finalize()
{

	object3dSignatureBlob->Release();
	if (object3dErrorBlob) {
		object3dErrorBlob->Release();
	}
	object3dPixelShaderBlob->Release();
	object3dVertexShaderBlob->Release();

}