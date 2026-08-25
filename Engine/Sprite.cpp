#include "Sprite.h"

void Sprite::Initialize()
{
	CreateVertexResource();
	CreateVertexData();
	CreateIndexResource();
	CreateIndexData();

	CreateMaterialResource();
	CreateWvpResource();
	CreateDirectionalLight();
}

void Sprite::CreateVertexResource()
{
	vertexResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount_);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount_;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Sprite::CreateVertexData()
{
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 単位サイズ（1x1）で作成（左上原点）
	vertexData[0].position = { 0.0f, 1.0f, 0.0f, 1.0f }; // 左下
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f }; // 右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[3].position = { 1.0f, 0.0f, 0.0f, 1.0f }; // 右上
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };
}

void Sprite::CreateIndexResource()
{
	indexResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * indexCount_);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount_;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void Sprite::CreateIndexData()
{
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
}

void Sprite::CreateMaterialResource()
{
	materialResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->lightingType = 0; // None
	materialData_->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateWvpResource()
{
	wvpResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = MakeIdentity4x4();
}

void Sprite::CreateDirectionalLight()
{
	directionalLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
}

// カメラなし版の Update（2D画面変換を自動生成）
void Sprite::Update(float screenWidth, float screenHeight)
{
	// 1. ワールド行列の計算 (サイズ適用)
	Transform currentTransform = transform; //[cite: 18]
	currentTransform.scale.x *= size.x; //[cite: 18]
	currentTransform.scale.y *= size.y; //[cite: 18]
	currentTransform.parent = nullptr;
	currentTransform.UpdateMatrix(); //

	// 2. 2D画面座標(左上原点 0,0〜screenWidth,screenHeight)用 正射影行列作成
	Matrix4x4 matWorld = currentTransform.matWorld; // ★ matWorld_ から matWorld へ変更[cite: 20]
	Matrix4x4 matView = MakeIdentity4x4();
	Matrix4x4 matProjection = MakeOrthographicMatrix(0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 100.0f);

	if (wvpData_) {
		wvpData_->WVP = Multiply(matWorld, Multiply(matView, matProjection));
	}

	// 3. マテリアル・UV情報の更新
	if (materialData_) {
		materialData_->color = this->color; //[cite: 18]
		materialData_->lightingType = 0; // ライティング無効

		Matrix4x4 uvMatScale = MakeScaleMatrix(uvTransform.scale); //[cite: 18]
		Matrix4x4 uvMatRot = MakeRotateZMatrix(uvTransform.rotate.z); //[cite: 18]
		Matrix4x4 uvMatTrans = MakeTranslateMatrix(uvTransform.translate); //[cite: 18]

		materialData_->uvTransform = Multiply(uvMatScale, Multiply(uvMatRot, uvMatTrans));
	}
}

// 既存のカメラあり版 Update
void Sprite::Update(Camera* camera)
{
	if (!camera) {
		Update(); // カメラが nullptr の場合は自動的に 2D更新へフォールバック
		return;
	}

	camera_ = camera;

	Transform currentTransform = transform; //[cite: 18]
	currentTransform.scale.x *= size.x; //[cite: 18]
	currentTransform.scale.y *= size.y; //[cite: 18]
	currentTransform.UpdateMatrix(); //[cite: 20]

	if (wvpData_ && camera_) {
		*wvpData_ = camera_->CalculateWVP2D(currentTransform);
	}

	if (materialData_) {
		materialData_->color = this->color; //[cite: 18]
		materialData_->lightingType = 0;

		Matrix4x4 uvMatScale = MakeScaleMatrix(uvTransform.scale); //[cite: 18]
		Matrix4x4 uvMatRot = MakeRotateZMatrix(uvTransform.rotate.z); //[cite: 18]
		Matrix4x4 uvMatTrans = MakeTranslateMatrix(uvTransform.translate); //[cite: 18]

		materialData_->uvTransform = Multiply(uvMatScale, Multiply(uvMatRot, uvMatTrans));
	}
}

void Sprite::Draw(TextureHandle textureHandle)
{
	// ★ リソースの存在チェック（未初期化の場合は描画スキップ）
	if (!vertexResource_ || !indexResource_ || !materialResource_ || !wvpResource_) {
		return;
	}

	auto commandList = dxCommon_->GetCommandList();
	if (!commandList) return;

	// 頂点・インデックスバッファのセット
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 定数バッファのセット
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	// ★ SRVハンドルの取得と検証
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle);

	// ハンドルのアドレスが 0 (nullptr) の場合は描画を安全に中断
	if (srvHandle.ptr == 0) {
		return;
	}

	commandList->SetGraphicsRootDescriptorTable(2, srvHandle);

	if (directionalLightResource_) {
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	}

	// 描画実行
	commandList->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);
}