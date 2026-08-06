#include "Model.h"

void Model::Initialize(const std::string& filename)
{
	modelData_ = LoadObjFile("resources", filename);

	CreateModelSphere();

	// 定数バッファの生成
	CreateMaterialResource();
	CreateWvpResource();
	CreateDirectionalLight();
}

void Model::CreateModelSphere()
{
	vertexCount_ = static_cast<uint32_t>(modelData_.vertices.size());
	vertexResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount_);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Model::CreateMaterialResource()
{
	materialResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->lightingType = 1; // LightType_Lambert
	materialData_->uvTransform = MakeIdentity4x4();
}

void Model::CreateWvpResource()
{
	wvpResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = MakeIdentity4x4();
}

void Model::CreateDirectionalLight()
{
	directionalLightResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));

	// 光の色（白）
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// 光の差し込む向き（斜め前上から照らすように設定）
	directionalLightData_->direction = { 0.5f, -0.7f, 0.5f };
	// 光の強度（明るさ）
	directionalLightData_->intensity = 1.0f;
}

// 1. 自身の this->transform を使って描画する関数
void Model::Draw(Camera* camera, TextureHandle textureHandle)
{
	Draw(this->transform, camera, textureHandle);
}

// 2. 渡された transform を使って描画する関数（★ここを修正★）
void Model::Draw(const Transform& transform, Camera* camera, TextureHandle textureHandle)
{
	// 【重要】引数で渡された transform の行列を更新する
	// （親の行列 parent が設定されている場合もこれで計算されます）
	Transform currentTransform = transform;
	currentTransform.UpdateMatrix();

	// WVP行列の計算と書き込み
	if (wvpData_ && camera) {
		*wvpData_ = camera->CalculateWVP(currentTransform);
	}

	if (materialData_) {
		materialData_->color = this->color;
		// ★テスト用にライティングを 0 (None) にして、テクスチャ本来の色が出るか確認
		materialData_->lightingType = 0; // LightType_None
	}

	// 描画コマンドの発行
	auto commandList = dxCommon_->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle);
	commandList->SetGraphicsRootDescriptorTable(2, srvHandle);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	// 描画呼出し
	commandList->DrawInstanced(vertexCount_, instanceCount_, 0, 0);
}

// --- 以下 LoadMaterialTemplateFile / LoadObjFile は元の処理のまま ---

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector2> texcoords;
	std::vector<Vector3> normals;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;

				VertexData vertex = { position, texcoord, normal };
				modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position, texcoord, normal };
			}
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;

			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}