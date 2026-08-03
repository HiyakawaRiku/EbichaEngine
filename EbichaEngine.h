#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "DebugRenderer.h"
#include "DebugCamera.h"
#include "Matrix.h"
#include "Triangle.h"
#include "Sprite.h"
#include "Camera.h"
#include "Model.h"
#include "Sphere.h"
#include "Audio.h"

#include "EMath.h"

class EbichaEngine
{
public:
	static EbichaEngine* GetInstance();
	void Initialize();
	void Finalize();
};