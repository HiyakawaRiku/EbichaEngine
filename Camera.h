#pragma once
#include "Matrix.h"
#include "WinApp.h"
#include "BaseObject.h"

class Camera
{
public:
	TransformationMatrix DrawObject3d(Transform transform);
	TransformationMatrix DrawObject2d(Transform transform);

	Transform transform_{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
private:
	WinApp* app_ = WinApp::GetInstance();
};

