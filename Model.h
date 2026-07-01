#pragma once
#include "BaseObject.h"

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

class Model:public BaseObject
{
public:
	void Initialize(const std::string& directoryPath, const std::string& filename);
	void Draw();

private:
	ModelData modelData_;

private:
	void CreateModelSphere();
};


MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);