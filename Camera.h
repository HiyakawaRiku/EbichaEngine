#pragma once
#include "Matrix.h"
#include "WinApp.h"

class Camera
{
public:
	Matrix4x4 DrawObject3d(Transform transform);
	Matrix4x4 DrawObject2d(Transform transform);

private:
	WinApp* app_ = WinApp::GetInstance();
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
};

