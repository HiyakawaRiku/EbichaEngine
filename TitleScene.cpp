#include "TitleScene.h"

void TitleScene::Initialize()
{
	finished_ = false;

	model_ = std::make_unique<TitleLogo>();
	model_->Initialize();

	activeCamera_ = std::make_unique<Camera>();
	activeCamera_->Initialize();
}

void TitleScene::Update()
{
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		finished_ = true;
	}
	activeCamera_->Update();

	if (model_) {
		model_->Update(activeCamera_.get());
	}
}

void TitleScene::Draw()
{
	DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kNormal, DepthWrite::kEnable);

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