#include "Model.h"
#include <algorithm>

void Model::Initialize(const std::string& filename)
{
	modelData_ = LoadModelFile("resources/" + filename, filename+".obj");

	CreateModelSphere();

	CreateMaterialResource();
	CreateWvpResource();
	CreateDirectionalLight();
	CreatePointLight();
	CreateSpotLight();

	CreateInstanceResource();
}

void Model::CreateModelSphere()
{
	vertexCount_ = static_cast<uint32_t>(modelData_.vertices.size());
	vertexResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount_);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Model::CreateMaterialResource()
{
	materialResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->lightingType = this->lightingType; // ★固定値 2 ではなくメンバ変数を参照
	materialData_->shininess = 50.0f; // ★Sphereに合わせて設定
	materialData_->uvTransform = MakeIdentity4x4();
}

void Model::CreateWvpResource()
{
	wvpResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = MakeIdentity4x4();
}

void Model::CreateDirectionalLight()
{
	directionalLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));

	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.5f, -0.7f, 0.5f };
	directionalLightData_->intensity = 1.0f;
}

void Model::CreateInstanceResource()
{
	instanceResource_ = DirectXUtils::CreateBufferResource(
		dxCommon_->GetDevice(),
		sizeof(ParticleForGPU) * kMaxInstanceCount
	);

	instanceResource_->Map(0, nullptr, reinterpret_cast<void**>(&instanceData_));

	for (uint32_t index = 0; index < kMaxInstanceCount; ++index) {
		instanceData_[index].WVP = MakeIdentity4x4();
		instanceData_[index].World = MakeIdentity4x4();
		instanceData_[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	dxCommon_->CreateInstancingSrv(
		instanceSrvIndex_,
		instanceResource_.Get(),
		kMaxInstanceCount,
		sizeof(ParticleForGPU)
	);
}

void Model::CreatePointLight()
{
	pointLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PointLight));
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));

	pointLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData_->position = { 0.0f, 2.0f, 0.0f };
	pointLightData_->intensity = 1.0f;
	pointLightData_->radius = 10.0f;
	pointLightData_->decay = 2.0f;
}

void Model::CreateSpotLight()
{
	spotLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(SpotLight));
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));

	spotLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData_->position = { 0.0f, 3.0f, 0.0f };
	spotLightData_->direction = { 0.0f, -1.0f, 0.0f };
	spotLightData_->intensity = 2.0f;
	spotLightData_->distance = 10.0f;
	spotLightData_->decay = 2.0f;
	spotLightData_->cosAngle = std::cos(DirectX::XMConvertToRadians(30.0f));
	spotLightData_->cosFalloffStart = std::cos(DirectX::XMConvertToRadians(15.0f));
}

void Model::Update(Camera* camera)
{
	Transform currentTransform = transform;
	currentTransform.UpdateMatrix();

	if (wvpData_ && camera) {
		//Matrix4x4 worldMatrix = currentTransform.matWorld;

		//Matrix4x4 viewMatrix = camera->GetViewMatrix();
		//Matrix4x4 projectionMatrix = camera->GetProjectionMatrix();
		//Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

		//Matrix4x4 rootWorld = Multiply(modelData_.rootNode.localMatrix, worldMatrix);

		//wvpData_->World = rootWorld;
		//wvpData_->WVP = Multiply(rootWorld, viewProjectionMatrix);
		*wvpData_ = camera->CalculateWVP(currentTransform);
	}

	if (materialData_) {
		materialData_->color = this->color;
		materialData_->lightingType = this->lightingType; // ★重要: 0 から this->lightingType に修正
	}

}

void Model::Draw(Camera* camera, TextureHandle textureHandle)
{
	Draw(this->transform, camera, textureHandle);
}

void Model::Draw(const Transform& transform, Camera* camera, TextureHandle textureHandle)
{
	if (!camera || !camera->GetCameraResource()) {
		return;
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

	if (camera && camera->GetCameraResource()) {
		commandList->SetGraphicsRootConstantBufferView(5, camera->GetCameraResource()->GetGPUVirtualAddress());
	}

	if (pointLightResource_) {
		commandList->SetGraphicsRootConstantBufferView(6, pointLightResource_->GetGPUVirtualAddress()); // b3
	}
	if (spotLightResource_) {
		commandList->SetGraphicsRootConstantBufferView(7, spotLightResource_->GetGPUVirtualAddress()); // b4
	}

	// 描画呼出し
	commandList->DrawInstanced(vertexCount_, instanceCount_, 0, 0);
}

void Model::DrawInstanced(std::list<ParticleData>& particles, Camera* camera, TextureHandle textureHandle)
{
	if (!camera || !camera->GetCameraResource()) {
		return;
	}

	if (particles.empty()) return;

	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeIdentity4x4();
	Matrix4x4 backToFrontMatrix = MakeIdentity4x4();
	if (camera) {
		viewMatrix = camera->GetViewMatrix();
		projectionMatrix = camera->GetProjectionMatrix();

		backToFrontMatrix = viewMatrix;
		backToFrontMatrix.m[3][0] = 0.0f;
		backToFrontMatrix.m[3][1] = 0.0f;
		backToFrontMatrix.m[3][2] = 0.0f;
		backToFrontMatrix = Inverse(backToFrontMatrix);
	}
	Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

	uint32_t numInstance = 0;

	for (auto& particle : particles) {

		if (numInstance < kMaxInstanceCount) {

			if (particle.lifeTime <= particle.currentTime) {
				continue;
			}

			// ビルボードを考慮したワールド行列の作成
			Matrix4x4 scaleMatrix = MakeScaleMatrix(particle.transform.scale);
			Matrix4x4 translateMatrix = MakeTranslateMatrix(particle.transform.translate);
			Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(backToFrontMatrix, translateMatrix));

			instanceData_[numInstance].World = worldMatrix;
			instanceData_[numInstance].WVP = Multiply(worldMatrix, viewProjectionMatrix);
			instanceData_[numInstance].color = particle.color;
			float alpha = 1.0f - (particle.currentTime / particle.lifeTime);
			instanceData_[numInstance].color.w = alpha;

			++numInstance;
		}
	}

	if (numInstance == 0) return;

	auto commandList = dxCommon_->GetCommandList();

	// 頂点バッファ・トポロジ設定[cite: 5]
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パイプライン・定数バッファ等のバインド[cite: 5]
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle);
	commandList->SetGraphicsRootDescriptorTable(2, srvHandle);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = dxCommon_->GetInstancingSrvHandleGPU(instanceSrvIndex_);
	commandList->SetGraphicsRootDescriptorTable(4, instancingSrvHandleGPU);

	if (camera && camera->GetCameraResource()) {
		commandList->SetGraphicsRootConstantBufferView(5, camera->GetCameraResource()->GetGPUVirtualAddress());
	}

	if (pointLightResource_) {
		commandList->SetGraphicsRootConstantBufferView(6, pointLightResource_->GetGPUVirtualAddress()); // b3
	}
	if (spotLightResource_) {
		commandList->SetGraphicsRootConstantBufferView(7, spotLightResource_->GetGPUVirtualAddress()); // b4
	}

	commandList->DrawInstanced(UINT(modelData_.vertices.size()), numInstance, 0, 0);
}

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

ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename) {

	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector2> texcoords;
	std::vector<Vector3> normals;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y };
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);
			}
		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

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

Node ReadNode(aiNode* node)
{
	Node result;
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;
	aiLocalMatrix.Transpose();
	result.localMatrix.m[0][0] = aiLocalMatrix[0][0];
	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}