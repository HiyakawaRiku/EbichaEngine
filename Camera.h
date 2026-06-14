#pragma once
#include "Matrix.h"
#include "WinApp.h"
#include "Mesh.h"

class Camera
{
public:
	TransformationMatrix DrawObject3d(Transform transform);
	TransformationMatrix DrawObject2d(Transform transform);

private:
	WinApp* app_ = WinApp::GetInstance();
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
};

