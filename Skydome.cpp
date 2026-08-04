#include "Skydome.h"

void Skydome::Initialize()
{
	model_ = new Model();
	model_->Initialize("skydome.obj");
	textureHandle_ = TextureManager::GetInstance()->Load("resources/sky_sphere.png", DirectXCommon::GetInstance()->GetCommandList());
}

void Skydome::Update(Camera* activeCamera_)
{
	model_->Update(activeCamera_);
}

void Skydome::Draw()
{
	model_->Draw(textureHandle_);
}
