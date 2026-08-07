#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "DebugRenderer.h"
#include "DebugCamera.h"
#include "Mesh.h"
#include "Sprite.h"
#include "Camera.h"
#include "Model.h"
#include "Sphere.h"
#include "Audio.h"

#include "EMath.h"
#include "Transform.h"
#include "TextureManager.h"

class EbichaEngine
{
public:
	static EbichaEngine* GetInstance();
	void Initialize();
	void Finalize();
};