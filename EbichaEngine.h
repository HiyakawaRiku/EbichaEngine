#pragma once
#include "DirectXCommon.h"
#include "Matrix.h"
#include "Triangle.h"
#include "Sprite.h"
#include "Camera.h"
#include "Model.h"
#include "Sphere.h"

#include "DebugRenderer.h"

class EbichaEngine
{
public:
	static EbichaEngine* GetInstance();
	void Initialize();
	void Finalize();
};

