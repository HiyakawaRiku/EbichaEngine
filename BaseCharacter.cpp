#include "BaseCharacter.h"

void BaseCharacter::Initialize(const std::vector<Model*>& models)
{
	models_ = models;
	transform_.Initialize();
}

void BaseCharacter::Update(const Camera& activeCamera_)
{
	transform_.UpdateMatrix();
}

//void BaseCharacter::Draw()
//{
//	for (Model* model : models_) {
//		model->Draw()
//	}
//}
