#pragma once
#include "DirectXCommon.h"
#include "Model.h"
#include <map>

class ModelManager final
{
public:
	static ModelManager* GetInstance();
	void Initialize(DirectXCommon* dxCommon);
	void LoadModel(const std::string& filePath);
	Model* FindModel(const std::string& filePath);

private:
	static ModelManager* instance;

	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager& obj) = delete;
	ModelManager& operator=(const ModelManager& obj) = delete;

	private:
	std::map<std::string, std::unique_ptr<Model>> models;

	std::unique_ptr<ModelCommon> modelCommon = nullptr;
};

