#pragma once
#include "BaseObject.h"

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

