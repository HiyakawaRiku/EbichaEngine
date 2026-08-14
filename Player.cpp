#include "Player.h"
#include <cmath>

void Player::Initialize()
{
    // パーツ構成を定義
    std::vector<BaseCharacter::PartConfig> partConfigs = {
        { "Head.obj",  {  0.0f,  0.5f, 0.0f } },
        { "armL1.obj", { -0.5f,  0.5f, 0.0f } },
        { "armR1.obj", {  0.5f,  0.5f, 0.0f } },
        { "legL1.obj", { -0.2f, -0.5f, 0.0f } },
        { "legR1.obj", {  0.2f, -0.5f, 0.0f } },
    };

    // 親クラスの初期化関数に渡す[cite: 5]
    BaseCharacter::Initialize("Body.obj", partConfigs, "resources/tex.png"); //[cite: 5]
    transformBase_.translate = { 0.0f, 1.5f, 0.0f }; 

    InitializeFloatingGimmick(); 

    // コライダーの半径を設定（例: 1.0f）
    SetColliderRadius(1.0f);
}

void Player::Update(Camera* activeCamera)
{
    // 親クラスの更新（カメラセット、パーツ更新、行列更新）を実行[cite: 5]
    BaseCharacter::Update(activeCamera);

    // プレイヤー固有の挙動更新[cite: 9]
    BehaviorRootUpdate(activeCamera);

    // ImGui によるパラメータ調整ウィンドウ[cite: 9]
    ImGui::Begin("Player Jump Settings"); 
    ImGui::SliderFloat("Initial Velocity", &jumpInitialVelocity_, 0.1f, 1.5f); 
    ImGui::SliderFloat("Gravity", &gravity_, 0.005f, 0.1f); 
    ImGui::SliderFloat("Squash Amount", &jumpSquashAmount_, 0.0f, 0.6f); 
    ImGui::SliderFloat("Ground Y", &jumpGroundY_, 0.0f, 5.0f); 

    if (ImGui::Button("Test Jump") && !isJumping_) { 
        BehaviorJumpInitialize(); 
    }
    ImGui::End(); 
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