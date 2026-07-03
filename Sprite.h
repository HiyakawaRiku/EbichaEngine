#pragma once
#include "BaseObject.h"

class Sprite:public BaseObject{
public:
	void Initialize()override;
private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateIndexResource();
	void CreateIndexData();
};