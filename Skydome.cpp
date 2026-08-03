#include "Skydome.h"

void Skydome::Initialize()
{
	model_ = new Model();
	model_->Initialize("resources", "skydome.obj");
}

void Skydome::Update(Camera* activeCamera_)
{
	model_->Update(activeCamera_);
}

void Skydome::Draw()
{
	model_->Draw(3);
}
