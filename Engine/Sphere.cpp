#include "Sphere.h"

void Sphere::Initialize()
{
	CreateVertexResource();
	CreateVertexData();
	CreateIndexResource();
	CreateIndexData();

	CreateMaterialResource();
	CreateWvpResource();
	CreateDirectionalLight();
}

void Sphere::CreateVertexResource()
{
	vertexCount_ = (kSubdivision + 1) * (kSubdivision + 1);
	vertexResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount_);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount_;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Sphere::CreateVertexData()
{
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	uint32_t vertexIndex = 0;
	for (uint32_t lat = 0; lat <= kSubdivision; ++lat) {
		float theta = static_cast<float>(M_PI) * lat / kSubdivision;
		float sinTheta = sinf(theta);
		float cosTheta = cosf(theta);

		for (uint32_t lon = 0; lon <= kSubdivision; ++lon) {
			float phi = 2.0f * static_cast<float>(M_PI) * lon / kSubdivision;
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);

			Vector4 position = {
				sinTheta * cosPhi,
				cosTheta,
				sinTheta * sinPhi,
				1.0f
			};

			Vector3 normal = { position.x, position.y, position.z };

			Vector2 texcoord = {
				static_cast<float>(lon) / kSubdivision,
				static_cast<float>(lat) / kSubdivision
			};

			vertexData[vertexIndex] = { position, texcoord, normal };
			vertexIndex++;
		}
	}
	vertexResource_->Unmap(0, nullptr);
}

void Sphere::CreateIndexResource()
{
	indexCount_ = kSubdivision * kSubdivision * 6;
	indexResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * indexCount_);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount_;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void Sphere::CreateIndexData()
{
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	uint32_t indexOffset = 0;
	for (uint32_t lat = 0; lat < kSubdivision; ++lat) {
		for (uint32_t lon = 0; lon < kSubdivision; ++lon) {
			uint32_t p00 = lat * (kSubdivision + 1) + lon;
			uint32_t p01 = p00 + 1;
			uint32_t p10 = (lat + 1) * (kSubdivision + 1) + lon;
			uint32_t p11 = p10 + 1;

			indexData[indexOffset++] = p00;
			indexData[indexOffset++] = p01;
			indexData[indexOffset++] = p10;

			indexData[indexOffset++] = p01;
			indexData[indexOffset++] = p11;
			indexData[indexOffset++] = p10;
		}
	}
	indexResource_->Unmap(0, nullptr);
}

void Sphere::CreateMaterialResource()
{
	materialResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->lightingType = 2; // Lambert有効
	materialData_->shininess = 50.0f;
	materialData_->uvTransform = MakeIdentity4x4();
}

void Sphere::CreateWvpResource()
{
	wvpResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = MakeIdentity4x4();
}

void Sphere::CreateDirectionalLight()
{
	directionalLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.5f, -0.7f, 0.5f };
	directionalLightData_->intensity = 1.0f;
}

void Sphere::Update(Camera* camera)
{
	camera_ = camera;

	// 行列更新と3D用WVP計算
	transform.UpdateMatrix();

	if (wvpData_ && camera_) {
		*wvpData_ = camera_->CalculateWVP(transform);
	}

	if (materialData_) {
		materialData_->color = this->color;
		materialData_->lightingType = this->lightingType;

		Matrix4x4 uvMatScale = MakeScaleMatrix(uvTransform.scale);
		Matrix4x4 uvMatRot = MakeRotateZMatrix(uvTransform.rotate.z);
		Matrix4x4 uvMatTrans = MakeTranslateMatrix(uvTransform.translate);
		materialData_->uvTransform = Multiply(uvMatScale, Multiply(uvMatRot, uvMatTrans));
	}
}

void Sphere::Draw(TextureHandle textureHandle)
{
	ImGui::Begin("pos");
	ImGui::DragFloat3("pos", &directionalLightData_->direction.x, 0.1f);
	ImGui::End();

	auto commandList = dxCommon_->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle);
	commandList->SetGraphicsRootDescriptorTable(2, srvHandle);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	if (camera_) {
		commandList->SetGraphicsRootConstantBufferView(5, camera_->GetCameraResource()->GetGPUVirtualAddress());
	}

	commandList->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);
}