#pragma once
#include "BaseObject.h"

class Sphere :public BaseObject
{
public:
	void Initialize()override;

	uint32_t kSubdivision = 16;

private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateIndexResource();
	void CreateIndexData();

};

