#pragma once
#include "BaseObject.h"

class Sprite:public BaseObject{
public:
	void Initialize()override;
	void Update(class Camera* camera)override;
private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateIndexResource();
	void CreateIndexData();
};