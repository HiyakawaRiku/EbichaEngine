#include "TitleLogo.h"

TitleLogo::~TitleLogo()
{
	delete model_;
	model_ = nullptr;
}

void TitleLogo::Initialize()
{
	model_ = new Model();
	model_->Initialize("titleLogo");
	textureHandle_ = TextureManager::GetInstance()->Load("resources/sky_spahere.png", DirectXCommon::GetInstance()->GetCommandList());
}

void TitleLogo::Update(Camera* activeCamera)
{
	// カメラ参照を保持
	activeCamera_ = activeCamera;
	model_->Update(activeCamera);
	// 旧 BaseObject の Update(activeCamera_) 呼出しは不要になったため削除
}

void TitleLogo::Draw()
{
	if (model_) {
		// 新しい Model::Draw(Camera*, TextureHandle) を呼び出す
		model_->Draw(activeCamera_, textureHandle_);
	}
}