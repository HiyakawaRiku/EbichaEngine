#include "ClearScene.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "SceneManager.h"
#include "Input.h"
#include "Editor.h"

void ClearScene::Initialize() {
    sprite_ = Sprite::Create("Resources/monsterBall.png");
    sprite_->SetAnchorPoint({ 0.5f, 0.5f });
    sprite_->SetPosition({ 640.0f, 360.0f });

    Editor::GetInstance()->Register("Clear Sprite", sprite_.get());
}

void ClearScene::Update(Input* input) {
    sprite_->Update();

    // Enter でタイトルへ
    if (input->TriggerKey(DIK_RETURN)) {
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }
}

void ClearScene::Draw() {
    SpriteCommon::GetInstance()->CommonDrawSetting();
    sprite_->Draw();
}

void ClearScene::Finalize() {
    sprite_.reset();
}