#include "GameOverScene.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "SceneManager.h"
#include "Input.h"

void GameOverScene::Initialize() {
    uint32_t textureIndex = TextureManager::GetInstance()->Load("Resources/uvChecker.png");

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize(SpriteCommon::GetInstance(), textureIndex);
    sprite_->SetAnchorPoint({ 0.5f, 0.5f });
    sprite_->SetPosition({ 640.0f, 360.0f });
    sprite_->SetColor({ 1.0f, 0.3f, 0.3f, 1.0f });   // 赤っぽくする
}

void GameOverScene::Update(Input* input) {
    sprite_->Update();

    // Enter でリトライ（ゲームをもう一度）
    if (input->TriggerKey(DIK_RETURN)) {
        SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }
    // Esc でタイトルへ
    else if (input->TriggerKey(DIK_ESCAPE)) {
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }
}

void GameOverScene::Draw() {
    SpriteCommon::GetInstance()->CommonDrawSetting();
    sprite_->Draw();
}

void GameOverScene::Finalize() {
    sprite_.reset();
}