#pragma once
#include "BaseObject.h"

class Triangle :public BaseObject{
public:
	void Initialize(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2);

private:
	void CreateVertexResource();
	void CreateVertexData(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2);
};