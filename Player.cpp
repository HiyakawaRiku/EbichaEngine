#include "Player.h"
#include <cmath>

void Player::Initialize()
{
    // パーツ構成を定義
    std::vector<BaseCharacter::PartConfig> partConfigs = {
        { "Head",  {  0.0f,  0.5f, 0.0f } },
        { "armL1", { -0.5f,  0.5f, 0.0f } },
        { "armR1", {  0.5f,  0.5f, 0.0f } },
        { "legL1", { -0.2f, -0.5f, 0.0f } },
        { "legR1", {  0.2f, -0.5f, 0.0f } },
    };

    // 親クラスの初期化関数に渡す
    BaseCharacter::Initialize("Body", partConfigs, "resources/white1x1.png");
    transformBase_.translate = { 0.0f, 1.5f, 0.0f };

    InitializeFloatingGimmick();

    // コライダーの半径を設定
    SetColliderRadius(1.0f);

    // ★ HP & 無敵状態の初期化
    hp_ = kMaxHp_;
    isInvincible_ = false;
    invincibleTimer_ = 0.0f;
}

void Player::Update(Camera* activeCamera)
{
    // 親クラスの更新（カメラセット、パーツ更新、行列更新）を実行
    BaseCharacter::Update(activeCamera);

    // ★ 無敵時間のカウントダウン処理
    if (isInvincible_) {
        invincibleTimer_ -= 1.0f;
        if (invincibleTimer_ <= 0.0f) {
            isInvincible_ = false;
            invincibleTimer_ = 0.0f;
        }
    }

    // プレイヤー固有の挙動更新
    BehaviorRootUpdate(activeCamera);

    // ★ ImGui による HP ステータス表示・テストデバッグ用ウィンドウ
#ifdef _DEBUG
    ImGui::Begin("Player Status");
    ImGui::Text("HP: %d / %d", hp_, kMaxHp_);
    ImGui::ProgressBar((float)hp_ / (float)kMaxHp_, ImVec2(0.0f, 0.0f));
    ImGui::Text("Invincible: %s (Timer: %.0f)", isInvincible_ ? "TRUE" : "FALSE", invincibleTimer_);

    if (ImGui::Button("Take 1 Damage")) {
        TakeDamage(1);
    }
    if (ImGui::Button("Heal Full")) {
        hp_ = kMaxHp_;
    }
    ImGui::End();
#endif
}

// ★ 被弾ダメージ処理
void Player::TakeDamage(int damage)
{
    // 無敵中または既に死亡している場合は処理をスキップ
    if (isInvincible_ || IsDead()) {
        return;
    }

    // ダメージ減算
    hp_ -= damage;
    if (hp_ < 0) {
        hp_ = 0;
    }

    // 無敵時間を付与（連続ヒット防止）
    isInvincible_ = true;
    invincibleTimer_ = kInvincibleTime;

    // ※ここに被弾ノックバックやSE再生などを追加可能です
}

void Player::InitializeFloatingGimmick()
{
    floatingParameter_ = 0.0f; 
}

void Player::UpdateFloatingGimmick()
{
    const uint16_t cycle = (uint16_t)frame_; 
    const float step = 2.0f * 3.14f / cycle; 

    floatingParameter_ += step; 
    floatingParameter_ = std::fmod(floatingParameter_, 2.0f * 3.14f); 

    modelBody_->transform.translate.y = std::sin(floatingParameter_) * floatingAmplitude; 

    ImGui::Begin("Player"); 
    ImGui::SliderFloat("amplitude", &floatingAmplitude, 0.1f, 1.0f); 
    ImGui::End(); 
}

void Player::BehaviorRootUpdate(Camera* activeCamera_)
{
    if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isJumping_) { 
        BehaviorJumpInitialize(); 
    }

    if (isJumping_) { 
        BehaviorJumpUpdate(); 
    }

    // ★ 攻撃トリガーの追加（マウス左クリック または Fキー）
    if ((Input::GetInstance()->TriggerKey(DIK_F)) && !isAttacking_) {
        BehaviorAttackInitialize();
    }

    // ★ 攻撃中の更新処理
    if (isAttacking_) {
        BehaviorAttackUpdate();
    }

    Vector3 velocity_ = {}; 
    if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A) || Input::GetInstance()->PushKey(DIK_W) || Input::GetInstance()->PushKey(DIK_S)) { 

        Vector3 acceleration{}; 
        if (Input::GetInstance()->PushKey(DIK_D)) { acceleration.x += kAcceleration; } 
        else if (Input::GetInstance()->PushKey(DIK_A)) { acceleration.x -= kAcceleration; } 
        else if (Input::GetInstance()->PushKey(DIK_W)) { acceleration.z += kAcceleration; } 
        else if (Input::GetInstance()->PushKey(DIK_S)) { acceleration.z -= kAcceleration; } 
        velocity_ += acceleration; 
    }

    float moveX = Input::GetInstance()->GetLeftStickX(); 
    float moveZ = Input::GetInstance()->GetLeftStickY(); 
    Vector3 localMove = { moveX + velocity_.x, 0.0f, moveZ + velocity_.z }; 

    bool isMoving = (localMove.x != 0.0f || localMove.z != 0.0f); 

    if (isMoving) { 
        walkTimer_ += kWalkSpeed; 
        float swing = std::sin(walkTimer_) * kWalkAngle; 

        if (!isAttacking_ && modelParts_.size() >= 5) {
            modelParts_[1]->transform.rotate.x = swing;  // 左腕[cite: 9]
            modelParts_[2]->transform.rotate.x = -swing; // 右腕[cite: 9]
            modelParts_[3]->transform.rotate.x = -swing; // 左脚[cite: 9]
            modelParts_[4]->transform.rotate.x = swing;  // 右脚[cite: 9]
        }

        Vector3 worldMove = localMove; 
        if (activeCamera_) { 
            float cameraRotateY = activeCamera_->transform_.rotate.y; 
            Matrix4x4 matRotateY = MakeRotateYMatrix(cameraRotateY); 
            worldMove = TransformNormal(localMove, matRotateY); 
        }

        if (!isAttacking_) { 
            float targetAngle = std::atan2(worldMove.x, worldMove.z); 
            modelBody_->transform.rotate.y = EMath::LerpShortAngle( 
                modelBody_->transform.rotate.y, 
                targetAngle, 
                kRotateSpeed 
            );
        }

        transformBase_.translate += worldMove; 
    }
    else { 
        walkTimer_ = 0.0f; 
        idleTimer_ += kIdleSpeed; 
        float idleSin = std::sin(idleTimer_); 

        modelBody_->transform.translate.y = idleSin * kIdleBreathing; 

        for (size_t i = 1; i < modelParts_.size(); ++i) { 
            if ((i == 1 || i == 2) && isAttacking_) continue; 
            modelParts_[i]->transform.rotate.x = EMath::Lerp(modelParts_[i]->transform.rotate.x, 0.0f, 0.2f); 
        }

        if (!isAttacking_ && modelParts_.size() >= 3) { 
            modelParts_[1]->transform.rotate.z = idleSin * kIdleArmAngle;  // 左腕[cite: 9]
            modelParts_[2]->transform.rotate.z = -idleSin * kIdleArmAngle; // 右腕[cite: 9]
        }
    }
}

void Player::BehaviorJumpInitialize()
{
    isJumping_ = true; 
    jumpTimer_ = 0.0f; 
    jumpVelocityY_ = jumpInitialVelocity_; 

    modelBody_->transform.scale = { 
        1.0f + jumpSquashAmount_, 
        1.0f - jumpSquashAmount_, 
        1.0f + jumpSquashAmount_ 
    };
}

void Player::BehaviorJumpUpdate()
{
    if (!isJumping_) return; 

    jumpTimer_ += 1.0f; 

    transformBase_.translate.y += jumpVelocityY_; 
    jumpVelocityY_ -= gravity_; 

    if (transformBase_.translate.y <= jumpGroundY_) { 
        transformBase_.translate.y = jumpGroundY_; 
        jumpVelocityY_ = 0.0f; 

        modelBody_->transform.scale.x = EMath::Lerp(modelBody_->transform.scale.x, 1.0f + jumpSquashAmount_, 0.3f); 
        modelBody_->transform.scale.y = EMath::Lerp(modelBody_->transform.scale.y, 1.0f - jumpSquashAmount_, 0.3f); 
        modelBody_->transform.scale.z = EMath::Lerp(modelBody_->transform.scale.z, 1.0f + jumpSquashAmount_, 0.3f); 

        for (size_t i = 1; i < modelParts_.size(); ++i) { 
            modelParts_[i]->transform.rotate.x = EMath::Lerp(modelParts_[i]->transform.rotate.x, 0.0f, 0.3f); 
            modelParts_[i]->transform.rotate.z = EMath::Lerp(modelParts_[i]->transform.rotate.z, 0.0f, 0.3f); 
        }

        if (jumpTimer_ > 12.0f) { 
            modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f }; 
            isJumping_ = false; 
        }
    }
    else { 
        modelBody_->transform.scale.x = EMath::Lerp(modelBody_->transform.scale.x, 1.0f - (jumpSquashAmount_ * 0.5f), 0.1f); 
        modelBody_->transform.scale.y = EMath::Lerp(modelBody_->transform.scale.y, 1.0f + (jumpSquashAmount_ * 0.5f), 0.1f); 
        modelBody_->transform.scale.z = EMath::Lerp(modelBody_->transform.scale.z, 1.0f - (jumpSquashAmount_ * 0.5f), 0.1f); 

        if (jumpVelocityY_ > 0.0f) { 
            if (modelParts_.size() >= 5) {
                modelParts_[1]->transform.rotate.z = EMath::Lerp(modelParts_[1]->transform.rotate.z, 0.6f, 0.2f);  
                modelParts_[2]->transform.rotate.z = EMath::Lerp(modelParts_[2]->transform.rotate.z, -0.6f, 0.2f); 
                modelParts_[3]->transform.rotate.x = EMath::Lerp(modelParts_[3]->transform.rotate.x, -0.4f, 0.2f); 
                modelParts_[4]->transform.rotate.x = EMath::Lerp(modelParts_[4]->transform.rotate.x, -0.4f, 0.2f); 
            }
        }
        else { 
            if (modelParts_.size() >= 5) {
                modelParts_[1]->transform.rotate.x = EMath::Lerp(modelParts_[1]->transform.rotate.x, 0.3f, 0.2f); 
                modelParts_[2]->transform.rotate.x = EMath::Lerp(modelParts_[2]->transform.rotate.x, 0.3f, 0.2f); 
                modelParts_[3]->transform.rotate.x = EMath::Lerp(modelParts_[3]->transform.rotate.x, 0.5f, 0.2f); 
                modelParts_[4]->transform.rotate.x = EMath::Lerp(modelParts_[4]->transform.rotate.x, 0.5f, 0.2f); 
            }
        }
    }
}

// ★ 攻撃初期化処理
void Player::BehaviorAttackInitialize()
{
    isAttacking_ = true;
    attackTimer_ = 0.0f;
}

// 攻撃更新処理（大振りメガトンパンチ）
void Player::BehaviorAttackUpdate()
{
    if (!isAttacking_) return;

    attackTimer_ += 1.0f;
    float progress = attackTimer_ / kAttackDuration;

    if (modelParts_.size() >= 3 && modelBody_) {
        if (progress < 0.25f) {
            // 【1. タメ動作 (0%～25%)】 
            // 体を少し後ろに引いて右腕を大きく後ろへ構える
            modelBody_->transform.rotate.y -= 0.05f; // 体を右に捻る
            modelParts_[2]->transform.rotate.x = EMath::Lerp(modelParts_[2]->transform.rotate.x, 1.2f, 0.3f); // 右腕を後ろに引く
            modelParts_[2]->transform.rotate.z = EMath::Lerp(modelParts_[2]->transform.rotate.z, 0.5f, 0.3f);
        }
        else if (progress < 0.55f) {
            // 【2. 振り抜き＆踏み込み (25%～55%)】 
            // プレイヤーを少し前に前進させつつ、右腕と体を前へ一気に振り抜く！

            // 踏み込み（プレイヤー自体を前進させる）
            Vector3 forward = { std::sin(modelBody_->transform.rotate.y), 0.0f, std::cos(modelBody_->transform.rotate.y) };
            transformBase_.translate += forward * 0.15f;

            // 体を逆に大きく捻り、腕を前方へぶん回す
            modelBody_->transform.rotate.y += 0.12f;
            modelParts_[2]->transform.rotate.x = EMath::Lerp(modelParts_[2]->transform.rotate.x, -2.2f, 0.5f); // 前方へ突き出し
            modelParts_[2]->transform.rotate.z = EMath::Lerp(modelParts_[2]->transform.rotate.z, -0.3f, 0.5f);
        }
        else {
            // 【3. 残心・戻り動作 (55%～100%)】 
            // 体と腕の回転をじわっと元の姿勢に戻す
            modelParts_[2]->transform.rotate.x = EMath::Lerp(modelParts_[2]->transform.rotate.x, 0.0f, 0.15f);
            modelParts_[2]->transform.rotate.z = EMath::Lerp(modelParts_[2]->transform.rotate.z, 0.0f, 0.15f);
        }
    }

    // 規定フレームに達したら終了
    if (attackTimer_ >= kAttackDuration) {
        isAttacking_ = false;
        attackTimer_ = 0.0f;
        if (modelParts_.size() >= 3) {
            modelParts_[2]->transform.rotate = { 0.0f, 0.0f, 0.0f }; // 回転リセット
        }
    }
}