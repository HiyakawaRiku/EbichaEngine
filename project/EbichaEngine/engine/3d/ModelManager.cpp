#include "ModelManager.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance()
{
	static ModelManager instance;
	return &instance;
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {
	modelCommon = std::make_unique<ModelCommon>();
	modelCommon->Initialize(dxCommon);
}
void ModelManager::LoadModel(const std::string& filePath)
{
	if (models.contains(filePath)) {
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon.get(), "resources", filePath);
	models.insert(std::make_pair(filePath, std::move(model)));


}
Model* ModelManager::FindModel(const std::string& filePath)
{
	if (models.contains(filePath)) {
		return models.at(filePath).get();
	}

	return nullptr;
}

void ModelManager::Finalize()
{
	// 保持しているモデルデータをすべて解放
	models.clear();
	modelCommon.reset();
}