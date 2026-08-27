#include "GameOverScene.h"

void GameOverScene::Initialize()
{
	//camera_ = std::make_unique<Camera>();
	//camera_->Initialize();

	finished_ = false;

    model_ = std::make_unique<GameOverLogo>();
    model_->Initialize();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();
}

void GameOverScene::Update()
{
	//camera_->Update();

        // フェード中でなければ入力判定
    if (!FadeManager::GetInstance()->IsFading() && !finished_) {
        if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
            // フェードアウト開始
            FadeManager::GetInstance()->StartFadeOut(0.5f);
        }
    }

    // フェードアウトが完了したらシーン終了フラグを立てる
    if (FadeManager::GetInstance()->IsFadeOutFinished()) {
        finished_ = true;
    }
    FadeManager::GetInstance()->Update();
    camera_->Update();

    if (model_) {
        model_->Update(camera_.get());
    }
}

void GameOverScene::Draw()
{
    if (model_) {
        model_->Draw();
    }
}

void GameOverScene::Finalize()
{
}