#include "TitleScene.h"

void TitleScene::Initialize()
{
	finished_ = false;

	model_ = std::make_unique<TitleLogo>();
	model_->Initialize();

	activeCamera_ = std::make_unique<Camera>();
	activeCamera_->Initialize();

    DirectXCommon::GetInstance()->SetBlendMode(blendMode_);
}

void TitleScene::Update()
{
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
    activeCamera_->Update();

    if (model_) {
        model_->Update(activeCamera_.get());
    }
}

void TitleScene::Draw()
{
	DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kAdd, DepthWrite::kEnable);

	if (model_) {
		model_->Draw();
	}


	DirectXCommon::GetInstance()->SetPipeline(PipelineType::kParticle, BlendMode::kAdd, DepthWrite::kDisable);

#ifdef _DEBUG
	DebugRenderer::Flush(activeCamera_.get());
#endif
}

void TitleScene::Finalize()
{
	// 必要に応じて個別のリソース解放を記述
}