#include "GameOverScene.h"

void GameOverScene::Initialize()
{
	//camera_ = std::make_unique<Camera>();
	//camera_->Initialize();

	finished_ = false;
}

void GameOverScene::Update()
{
	//camera_->Update();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		finished_ = true;
	}
}

void GameOverScene::Draw()
{
}

void GameOverScene::Finalize()
{
}