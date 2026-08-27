#include "TitleScene.h"

void TitleScene::Initialize()
{
	finished_ = false;

	model_ = std::make_unique<TitleLogo>();
	model_->Initialize();

	activeCamera_ = std::make_unique<Camera>();
	activeCamera_->Initialize();

    DirectXCommon::GetInstance()->SetBlendMode(blendMode_);

    uiTexture_ = TextureManager::GetInstance()->Load("resources/ground_snow.png", DirectXCommon::GetInstance()->GetCommandList());

    uiSprite_ = std::make_unique<Sprite>();
    uiSprite_->Initialize();
    uiSprite_->size = { 1280.0f, 720.0f };                 // 画像サイズに合わせて適宜調整してください
    //uiSprite_->transform.translate = { 320.0f, 180.0f, 0.0f }; // 画面中央付近に配置（座標は適宜調整）
    uiSprite_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
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

    if (uiSprite_) {
        uiSprite_->Update();
    }
}

void TitleScene::Draw()
{
        // FadeManagerと同じパイプライン設定を使用
        DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kNormal, DepthWrite::kDisable);
        uiSprite_->Draw(uiTexture_);
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