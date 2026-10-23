#include "GamePlayScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ModelCommon.h"
#include "Sound.h"
#include "EMath.h"
#include "Input.h"

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

GamePlayScene::~GamePlayScene() {
    if (!soundData1_.buffer.empty()) {
        Sound::GetInstance()->SoundUnload(&soundData1_);
    }
}

void GamePlayScene::Initialize() {
    // シングルトン経由で各種マネージャーの参照を取得して利用
    SpriteCommon* spriteCommon = SpriteCommon::GetInstance();
    Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
    ModelCommon* modelCommon = ModelCommon::GetInstance();

    // モデル読み込み
    ModelManager::GetInstance()->LoadModel("axis.obj");

    // テクスチャロード
    textureIndex_ = TextureManager::GetInstance()->Load("Resources/uvChecker.png");
    textureIndex2_ = TextureManager::GetInstance()->Load("Resources/monsterBall.png");

    // スプライト作成
    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize(spriteCommon, (useMonsterBall_) ? textureIndex2_ : textureIndex_);

    sprite2_ = std::make_unique<Sprite>();
    sprite2_->Initialize(spriteCommon, textureIndex2_);

    // 3Dモデル・オブジェクト作成
    model_ = std::make_unique<Model>();
    model_->Initialize(modelCommon, "resources", "plane.obj");

    camera_ = std::make_unique<Camera>();
    camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
    camera_->SetTranslate({ 0.0f, 0.0f, -100.0f });
    object3dCommon->SetDefaultCamera(camera_.get());
    ParticleManager::GetInstance()->SetCamera(camera_.get());

    object3d_ = std::make_unique<Object3d>();
    object3d_->Initialize(object3dCommon);
    object3d_->SetModel(model_.get());

    object3d2_ = std::make_unique<Object3d>();
    object3d2_->Initialize(object3dCommon);
    object3d2_->SetModel("axis.obj");

    // パーティクル作成
    ParticleManager::GetInstance()->CreateParticleGroup("smoke", "Resources/monsterBall.png");
    emitter_ = std::make_unique<ParticleEmitter>("smoke", Vector3{ 0.0f, 0.0f, 0.0f }, 10, 0.2f);

    // サウンド再生
    soundData1_ = Sound::GetInstance()->SoundLoadFile("Resources/420_long_BPM108.mp3");
    pSourceVoice_ = Sound::GetInstance()->SoundPlayWave(soundData1_);
}

void GamePlayScene::Update(Input* input) {
    if (input->TriggerKey(DIK_0)) {
        OutputDebugStringA("Hit 0\n");
    }

#ifdef USE_IMGUI
    ImGui::Checkbox("useMonsterBall", &useMonsterBall_);
#endif

    // エミッター・パーティクル更新
    emitter_->Update();
    ParticleManager::GetInstance()->Update();

    // カメラ更新
    camera_->Update();

    // 3Dオブジェクト更新
    Vector3 a = object3d_->GetTranslate();
    a -= {0, 0, 1};
    object3d_->SetTranslate(a);
    object3d_->Update();
    object3d2_->Update();

    // スプライト更新
    Vector2 position = sprite_->GetPosition();
    position += Vector2{ 0.1f, 0.1f };
    sprite_->SetPosition(position);

    Vector4 color = sprite_->GetColor();
    color.x += 0.01f;
    if (color.x > 1.0f) {
        color.x -= 1.0f;
    }
    sprite_->SetColor(color);

    Vector2 size = sprite_->GetSize();
    size.x += 0.1f;
    size.y += 0.1f;
    sprite_->SetSize(size);

    sprite_->SetAnchorPoint({ 0.5f, 0.5f });
    sprite_->SetTextureSize({ 10.0f, 10.0f });

    useMonsterBall_ ? sprite_->SetTextureIndex(textureIndex2_) : sprite_->SetTextureIndex(textureIndex_);

    sprite_->Update();
}

void GamePlayScene::Draw() {
    // 3Dオブジェクト描画
    Object3dCommon::GetInstance()->CommonDrawSetting();
    object3d_->Draw();
    object3d2_->Draw();

    // パーティクル描画
    ParticleManager::GetInstance()->Draw();

    // スプライト描画
    SpriteCommon::GetInstance()->CommonDrawSetting();
    sprite_->Draw();
}

void GamePlayScene::Finalize() {
    
    if (pSourceVoice_) {
        pSourceVoice_->Stop(0);
        pSourceVoice_->DestroyVoice();
        pSourceVoice_ = nullptr;
    }

    if (!soundData1_.buffer.empty()) {
        Sound::GetInstance()->SoundUnload(&soundData1_);
    }

    // 保持しているスマートポインタや ComPtr を明示的に解放
    sprite_.reset();
    sprite2_.reset();
    object3d_.reset();
    object3d2_.reset();
    model_.reset();
    emitter_.reset();
    camera_.reset();

}