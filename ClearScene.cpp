#include "ClearScene.h"

void ClearScene::Initialize()
{
	//camera_ = std::make_unique<Camera>();
	//camera_->Initialize();

	finished_ = false;
}

void ClearScene::Update()
{
	//camera_->Update();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		finished_ = true;
	}
}

void ClearScene::Draw()
{
}

void ClearScene::Finalize()
{
}