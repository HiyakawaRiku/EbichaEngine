#include "TitleScene.h"
#include "GamePlayScene.h"
#include "SceneManager.h"
#include "Input.h"

void TitleScene::Initialize() {
    SpriteCommon* spriteCommon = SpriteCommon::GetInstance();

    // テクスチャ読込（環境に合わせて変更してください）
    textureIndex_ = TextureManager::GetInstance()->Load("Resources/uvChecker.png");

    // タイトル表示用スプライト生成
    titleSprite_ = std::make_unique<Sprite>();
    titleSprite_->Initialize(spriteCommon, textureIndex_);
    titleSprite_->SetPosition({ 400.0f, 300.0f });
    titleSprite_->SetAnchorPoint({ 0.5f, 0.5f });
}

void TitleScene::Update(Input* input) {
    titleSprite_->Update();

    // ENTERキーまたはSPACEキーを押したら GamePlayScene へ遷移
    if (input->TriggerKey(DIK_RETURN) || input->TriggerKey(DIK_SPACE)) {
        SceneManager::GetInstance()->ChangeScene(std::make_unique<GamePlayScene>());
    }
}

void TitleScene::Draw() {
    SpriteCommon::GetInstance()->CommonDrawSetting();
    titleSprite_->Draw();
}

void TitleScene::Finalize() {
    titleSprite_.reset();
}