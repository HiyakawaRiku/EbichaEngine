#include "Object3d.h"
#include "Logger.h"

using namespace Logger;

void Object3d::Initialize(EDirectX* eDirectX)
{

	emitter.transform.translate = { 0.0f,0.0f,0.0f };
	emitter.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter.transform.scale = { 1.0f,1.0f,1.0f };
	emitter.count = 3;
	emitter.frequency = 0.5f;
	emitter.frequencyTime = 0.0f;

	accelerationField.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };

	//const float pi = std::numbers::pi_v<float>;

	//const uint32_t kSubdivision = 16;
	//const size_t kTotalVertices = kSubdivision * kSubdivision * 6;


	// モデル読み込み
	modelData = LoadObjFile("resources", "plane.obj");
	// 頂点リソースを作る
	vertexResource = eDirectX->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// 頂点バッファビューを作成する
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソースの先頭のアドレスから使う
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size()); // 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData); // 1頂点あたりのサイズ

	// 頂点リソースにデータを書き込む
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData)); // 書き込むためのアドレスを取得
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size()); // 頂点データをリソースにコピー


	//// 左下
	//vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	//vertexData[0].texcoord = { 0.0f, 1.0f };
	//// 上
	//vertexData[1].position = { 0.0f, 0.5f, 0.0f, 1.0f };
	//vertexData[1].texcoord = { 0.5f, 0.0f };
	//// 右下
	//vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	//vertexData[2].texcoord = { 1.0f, 1.0f };
	//// 左下2
	//vertexData[3].position = { -0.5f, -0.5f, 0.5f, 1.0f };
	//vertexData[3].texcoord = { 0.0f, 1.0f };
	//// 上2
	//vertexData[4].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	//vertexData[4].texcoord = { 0.5f, 0.0f };
	//// 右下2
	//vertexData[5].position = { 0.5f, -0.5f, -0.5f, 1.0f };
	//vertexData[5].texcoord = { 1.0f, 1.0f };




	//// 経度分割1つ分の角度 φ_d
	//const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//// 緯度分割1つ分の角度 θ_d
	//const float kLatEvery = pi / float(kSubdivision);
	//// 緯度の方向に分割
	//for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
	//	float lat = -pi / 2.0f + kLatEvery * latIndex;// θ

	//	float v0 = 1.0f - float(latIndex) / float(kSubdivision);       // 下側の緯度
	//	float v1 = 1.0f - float(latIndex + 1) / float(kSubdivision);   // 上側の緯度

	//	// 経度の方向に分割しながら線を描く
	//	for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
	//		uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
	//		float lon = lonIndex * kLonEvery;// φ

	//		float u0 = float(lonIndex) / float(kSubdivision);          // 左側の経度
	//		float u1 = float(lonIndex + 1) / float(kSubdivision);      // 右側の経度

	//		// 頂点にデータを入力する。基準点a
	//		vertexData[start + 0].position.x = cos(lat) * cos(lon);
	//		vertexData[start + 0].position.y = sin(lat);
	//		vertexData[start + 0].position.z = cos(lat) * sin(lon);
	//		vertexData[start + 0].position.w = 1.0f;
	//		vertexData[start + 0].texcoord = { u0, v0 };
	//		vertexData[start + 0].normal.x = vertexData[start + 0].position.x;
	//		vertexData[start + 0].normal.y = vertexData[start + 0].position.y;
	//		vertexData[start + 0].normal.z = vertexData[start + 0].position.z;
	//		// 頂点にデータを入力する。基準点b
	//		vertexData[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
	//		vertexData[start + 1].position.y = sin(lat + kLatEvery);
	//		vertexData[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
	//		vertexData[start + 1].position.w = 1.0f;
	//		vertexData[start + 1].texcoord = { u0, v1 };
	//		vertexData[start + 1].normal.x = vertexData[start + 1].position.x;
	//		vertexData[start + 1].normal.y = vertexData[start + 1].position.y;
	//		vertexData[start + 1].normal.z = vertexData[start + 1].position.z;
	//		// 頂点にデータを入力する。基準点c
	//		vertexData[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
	//		vertexData[start + 2].position.y = sin(lat);
	//		vertexData[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
	//		vertexData[start + 2].position.w = 1.0f;
	//		vertexData[start + 2].texcoord = { u1, v0 };
	//		vertexData[start + 2].normal.x = vertexData[start + 2].position.x;
	//		vertexData[start + 2].normal.y = vertexData[start + 2].position.y;
	//		vertexData[start + 2].normal.z = vertexData[start + 2].position.z;
	//		// 頂点にデータを入力する。基準点b
	//		vertexData[start + 3].position.x = cos(lat + kLatEvery) * cos(lon);
	//		vertexData[start + 3].position.y = sin(lat + kLatEvery);
	//		vertexData[start + 3].position.z = cos(lat + kLatEvery) * sin(lon);
	//		vertexData[start + 3].position.w = 1.0f;
	//		vertexData[start + 3].texcoord = { u0, v1 };
	//		vertexData[start + 3].normal.x = vertexData[start + 3].position.x;
	//		vertexData[start + 3].normal.y = vertexData[start + 3].position.y;
	//		vertexData[start + 3].normal.z = vertexData[start + 3].position.z;
	//		// 頂点にデータを入力する。基準点d
	//		vertexData[start + 4].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
	//		vertexData[start + 4].position.y = sin(lat + kLatEvery);
	//		vertexData[start + 4].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
	//		vertexData[start + 4].position.w = 1.0f;
	//		vertexData[start + 4].texcoord = { u1, v1 };
	//		vertexData[start + 4].normal.x = vertexData[start + 4].position.x;
	//		vertexData[start + 4].normal.y = vertexData[start + 4].position.y;
	//		vertexData[start + 4].normal.z = vertexData[start + 4].position.z;
	//		// 頂点にデータを入力する。基準点c
	//		vertexData[start + 5].position.x = cos(lat) * cos(lon + kLonEvery);
	//		vertexData[start + 5].position.y = sin(lat);
	//		vertexData[start + 5].position.z = cos(lat) * sin(lon + kLonEvery);
	//		vertexData[start + 5].position.w = 1.0f;
	//		vertexData[start + 5].texcoord = { u1, v0 };
	//		vertexData[start + 5].normal.x = vertexData[start + 5].position.x;
	//		vertexData[start + 5].normal.y = vertexData[start + 5].position.y;
	//		vertexData[start + 5].normal.z = vertexData[start + 5].position.z;
	//	}
	//}


	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = eDirectX->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();


	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource = eDirectX->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書きこんでおく
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	// Instancing用のTransformationMatrixリソースを作る
	instancingResource = eDirectX->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);
	// データを書き込む
	// 書き込むためのアドレスを取得
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
	// 単位行列を書きこんでおく
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		instancingData[index].WVP = MakeIdentity4x4();
		instancingData[index].World = MakeIdentity4x4();
		instancingData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}


	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	directionalLightResource = eDirectX->CreateBufferResource(sizeof(DirectionalLight));
	// データを書き込む
	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 単位行列を書きこんでおく
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;


	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC particleDescriptionRootSignature{};
	particleDescriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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
	D3D12_ROOT_PARAMETER particleRootParameters[4] = {};

	particleRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	particleRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;   // PixelShaderで使う
	particleRootParameters[0].Descriptor.ShaderRegister = 0;   // レジスタ番号0を使う
	particleRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	particleRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderを使う
	particleRootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing; // Tableの中身の配列を指定
	particleRootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing); // Tableで利用する数
	particleRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	particleRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	particleRootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	particleRootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	particleRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	particleRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	particleRootParameters[3].Descriptor.ShaderRegister = 1;   // レジスタ番号1を使う
	particleDescriptionRootSignature.pParameters = particleRootParameters;   // ルートパラメータ配列へのポインタ
	particleDescriptionRootSignature.NumParameters = _countof(particleRootParameters);   // 配列の長さ

	D3D12_STATIC_SAMPLER_DESC particleStaticSamplers[1] = {};
	particleStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	particleStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	particleStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	particleStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	particleStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	particleStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	particleStaticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	particleStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	particleDescriptionRootSignature.pStaticSamplers = particleStaticSamplers;
	particleDescriptionRootSignature.NumStaticSamplers = _countof(particleStaticSamplers);

	// シリアライズしてバイナリにする

	HRESULT hr = D3D12SerializeRootSignature(&particleDescriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &particleSignatureBlob, &particleErrorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(particleErrorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = eDirectX->GetDevice()->CreateRootSignature(0,
		particleSignatureBlob->GetBufferPointer(), particleSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&particleRootSignature));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC particleInputElementDescs[3] = {};
	particleInputElementDescs[0].SemanticName = "POSITION";
	particleInputElementDescs[0].SemanticIndex = 0;
	particleInputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	particleInputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	particleInputElementDescs[1].SemanticName = "TEXCOORD";
	particleInputElementDescs[1].SemanticIndex = 0;
	particleInputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	particleInputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	particleInputElementDescs[2].SemanticName = "COLOR";
	particleInputElementDescs[2].SemanticIndex = 0;
	particleInputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	particleInputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC particleInputLayoutDesc{};
	particleInputLayoutDesc.pInputElementDescs = particleInputElementDescs;
	particleInputLayoutDesc.NumElements = _countof(particleInputElementDescs);


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
	particleVertexShaderBlob = eDirectX->CompileShader(L"Particle.VS.hlsl",
		L"vs_6_0");
	assert(particleVertexShaderBlob != nullptr);

	particlePixelShaderBlob = eDirectX->CompileShader(L"Particle.PS.hlsl", L"ps_6_0");
	assert(particlePixelShaderBlob != nullptr);


	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC particleDepthStencilDesc{};
	// Depthの機能を有効化する
	particleDepthStencilDesc.DepthEnable = true;
	// 書き込みします
	particleDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	// 比較関数はLessEqual。つまり、近ければ描画される
	particleDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC particlePipelineStateDesc{};
	particlePipelineStateDesc.pRootSignature = particleRootSignature.Get(); // RootSignature
	particlePipelineStateDesc.InputLayout = particleInputLayoutDesc; // InputLayout
	particlePipelineStateDesc.VS = { particleVertexShaderBlob->GetBufferPointer(),
		particleVertexShaderBlob->GetBufferSize() }; // VertexShader
	particlePipelineStateDesc.PS = { particlePixelShaderBlob->GetBufferPointer(),
		particlePixelShaderBlob->GetBufferSize() }; // Pixel Shader
	particlePipelineStateDesc.BlendState = blendDesc[2]; // BlendState
	particlePipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	particlePipelineStateDesc.NumRenderTargets = 1;
	particlePipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	particlePipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定（気にしなくて良い）
	particlePipelineStateDesc.SampleDesc.Count = 1;
	particlePipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	particlePipelineStateDesc.DepthStencilState = particleDepthStencilDesc;
	particlePipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	hr = eDirectX->GetDevice()->CreateGraphicsPipelineState(&particlePipelineStateDesc,
		IID_PPV_ARGS(&particlePipelineState));
	assert(SUCCEEDED(hr));

}

void Object3d::Update()
{

	numInstance = 0;

	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;


	//Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(EWindow::kClientWidth) / float(EWindow::kClientHeight), 0.1f, 100.0f);
	//Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	//wvpData->World = worldViewProjectionMatrix;
	//wvpData->WVP = worldViewProjectionMatrix;

	emitter.frequencyTime += kDeltaTime; // 時刻を進める
	if (emitter.frequency <= emitter.frequencyTime) { // 頻度より大きいなら発生
		particles.splice(particles.end(), Emit(emitter, randomEngine)); // 発生処理
		emitter.frequencyTime -= emitter.frequency; // 余計に過ぎた時間も加味して頻度計算する
	}


	for (std::list<Particle>::iterator particleIterator = particles.begin(); particleIterator != particles.end();) {
		if (numInstance < kNumMaxInstance) {
			if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
				particleIterator = particles.erase(particleIterator);
				continue;
			}

			Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
			Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);

			Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(billboardMatrix, translateMatrix));
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

			if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
				(*particleIterator).velocity.x += accelerationField.acceleration.x * kDeltaTime;
				(*particleIterator).velocity.y += accelerationField.acceleration.y * kDeltaTime;
				(*particleIterator).velocity.z += accelerationField.acceleration.z * kDeltaTime;
			}

			(*particleIterator).transform.translate.x += (*particleIterator).velocity.x * kDeltaTime;
			(*particleIterator).transform.translate.y += (*particleIterator).velocity.y * kDeltaTime;
			(*particleIterator).transform.translate.z += (*particleIterator).velocity.z * kDeltaTime;
			(*particleIterator).currentTime += kDeltaTime;
			instancingData[numInstance].WVP = worldViewProjectionMatrix;// 10 <= numInstance || numInstance < 0はバッファオーバーラン
			instancingData[numInstance].World = worldMatrix;
			instancingData[numInstance].color = (*particleIterator).color;
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
			instancingData[numInstance].color.w = alpha;
			++numInstance;
		}
		++particleIterator;
	}
}

void Object3d::Draw(EDirectX* eDirectX, D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU, D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2, D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU)
{

	//// 描画用のDescriptorHeapの設定
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap.Get() };
	//commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
	//commandList->RSSetViewports(1, &viewport); // Viewportを設定
	//commandList->RSSetScissorRects(1, &scissorRect); // Scirssorを設定
	//// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	//commandList->SetGraphicsRootSignature(rootSignature.Get());
	//commandList->SetPipelineState(graphicsPipelineState.Get()); // PSOを設定
	//commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	//// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	//commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//// マテリアルCBufferの場所を設定
	//commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//// wvp用のCBufferの場所を設定
	//commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	//// SRVのDescriptorTableの先頭を設定。2はrootParameters[2]である。
	//commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
	//// directionalLight用のCBufferの場所を設定
	//commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	//// 描画！（DrawCall/ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
	//commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);



	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	eDirectX->GetCommandList()->SetGraphicsRootSignature(particleRootSignature.Get());
	eDirectX->GetCommandList()->SetPipelineState(particlePipelineState.Get()); // PSOを設定
	eDirectX->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	eDirectX->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// マテリアルCBufferの場所を設定
	eDirectX->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// instancing用のDataを読むためにStructuredBufferのSRVを設定する
	eDirectX->GetCommandList()->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU);
	// SRVのDescriptorTableの先頭を設定。2はrootParameters[2]である。
	eDirectX->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
	// directionalLight用のCBufferの場所を設定
	eDirectX->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// 描画！6頂点の板ポリゴンを、kNumInstance(今回は10)だけInstance描画を行う
	if (numInstance > 0) {
		eDirectX->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), numInstance, 0, 0);
	}
}

void Object3d::Finalize()
{
	particleSignatureBlob->Release();
	if (particleErrorBlob) {
		particleErrorBlob->Release();
	}
	particlePixelShaderBlob->Release();
	particleVertexShaderBlob->Release();

}