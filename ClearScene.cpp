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
}

void ClearScene::Draw()
{
}

void ClearScene::Finalize()
{
}