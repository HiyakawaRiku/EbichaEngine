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
    transformBase_.translate = { 0.0f, 1.5f, 0.0f }; //[cite: 9]

    InitializeFloatingGimmick(); //[cite: 9]
}

void Player::Update(Camera* activeCamera)
{
    // 親クラスの更新（カメラセット、パーツ更新、行列更新）を実行[cite: 5]
    BaseCharacter::Update(activeCamera);

    // プレイヤー固有の挙動更新[cite: 9]
    BehaviorRootUpdate(activeCamera);

    // ImGui によるパラメータ調整ウィンドウ[cite: 9]
    ImGui::Begin("Player Jump Settings"); //[cite: 9]
    ImGui::SliderFloat("Initial Velocity", &jumpInitialVelocity_, 0.1f, 1.5f); //[cite: 9]
    ImGui::SliderFloat("Gravity", &gravity_, 0.005f, 0.1f); //[cite: 9]
    ImGui::SliderFloat("Squash Amount", &jumpSquashAmount_, 0.0f, 0.6f); //[cite: 9]
    ImGui::SliderFloat("Ground Y", &jumpGroundY_, 0.0f, 5.0f); //[cite: 9]

    if (ImGui::Button("Test Jump") && !isJumping_) { //[cite: 9]
        BehaviorJumpInitialize(); //[cite: 9]
    }
    ImGui::End(); //[cite: 9]
}

void Player::InitializeFloatingGimmick()
{
    floatingParameter_ = 0.0f; //[cite: 9]
}

void Player::UpdateFloatingGimmick()
{
    const uint16_t cycle = (uint16_t)frame_; //[cite: 9]
    const float step = 2.0f * 3.14f / cycle; //[cite: 9]

    floatingParameter_ += step; //[cite: 9]
    floatingParameter_ = std::fmod(floatingParameter_, 2.0f * 3.14f); //[cite: 9]

    modelBody_->transform.translate.y = std::sin(floatingParameter_) * floatingAmplitude; //[cite: 9]

    ImGui::Begin("Player"); //[cite: 9]
    ImGui::SliderFloat("amplitude", &floatingAmplitude, 0.1f, 1.0f); //[cite: 9]
    ImGui::End(); //[cite: 9]
}

void Player::BehaviorRootUpdate(Camera* activeCamera_)
{
    if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isJumping_) { //[cite: 9]
        BehaviorJumpInitialize(); //[cite: 9]
    }

    if (isJumping_) { //[cite: 9]
        BehaviorJumpUpdate(); //[cite: 9]
    }

    Vector3 velocity_ = {}; //[cite: 9]
    if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A) || Input::GetInstance()->PushKey(DIK_W) || Input::GetInstance()->PushKey(DIK_S)) { //[cite: 9]

        Vector3 acceleration{}; //[cite: 9]
        if (Input::GetInstance()->PushKey(DIK_D)) { acceleration.x += kAcceleration; } //[cite: 9]
        else if (Input::GetInstance()->PushKey(DIK_A)) { acceleration.x -= kAcceleration; } //[cite: 9]
        else if (Input::GetInstance()->PushKey(DIK_W)) { acceleration.z += kAcceleration; } //[cite: 9]
        else if (Input::GetInstance()->PushKey(DIK_S)) { acceleration.z -= kAcceleration; } //[cite: 9]
        velocity_ += acceleration; //[cite: 9]
    }

    float moveX = Input::GetInstance()->GetLeftStickX(); //[cite: 9]
    float moveZ = Input::GetInstance()->GetLeftStickY(); //[cite: 9]
    Vector3 localMove = { moveX + velocity_.x, 0.0f, moveZ + velocity_.z }; //[cite: 9]

    bool isMoving = (localMove.x != 0.0f || localMove.z != 0.0f); //[cite: 9]

    if (isMoving) { //[cite: 9]
        walkTimer_ += kWalkSpeed; //[cite: 9]
        float swing = std::sin(walkTimer_) * kWalkAngle; //[cite: 9]

        if (!isAttacking_ && modelParts_.size() >= 5) {
            modelParts_[1]->transform.rotate.x = swing;  // 左腕[cite: 9]
            modelParts_[2]->transform.rotate.x = -swing; // 右腕[cite: 9]
            modelParts_[3]->transform.rotate.x = -swing; // 左脚[cite: 9]
            modelParts_[4]->transform.rotate.x = swing;  // 右脚[cite: 9]
        }

        Vector3 worldMove = localMove; //[cite: 9]
        if (activeCamera_) { //[cite: 9]
            float cameraRotateY = activeCamera_->transform_.rotate.y; //[cite: 9]
            Matrix4x4 matRotateY = MakeRotateYMatrix(cameraRotateY); //[cite: 9]
            worldMove = TransformNormal(localMove, matRotateY); //[cite: 9]
        }

        if (!isAttacking_) { //[cite: 9]
            float targetAngle = std::atan2(worldMove.x, worldMove.z); //[cite: 9]
            modelBody_->transform.rotate.y = EMath::LerpShortAngle( //[cite: 9]
                modelBody_->transform.rotate.y, //[cite: 9]
                targetAngle, //[cite: 9]
                kRotateSpeed //[cite: 9]
            );
        }

        transformBase_.translate += worldMove; //[cite: 9]
    }
    else { //[cite: 9]
        walkTimer_ = 0.0f; //[cite: 9]
        idleTimer_ += kIdleSpeed; //[cite: 9]
        float idleSin = std::sin(idleTimer_); //[cite: 9]

        modelBody_->transform.translate.y = idleSin * kIdleBreathing; //[cite: 9]

        for (size_t i = 1; i < modelParts_.size(); ++i) { //[cite: 9]
            if ((i == 1 || i == 2) && isAttacking_) continue; //[cite: 9]
            modelParts_[i]->transform.rotate.x = EMath::Lerp(modelParts_[i]->transform.rotate.x, 0.0f, 0.2f); //[cite: 9]
        }

        if (!isAttacking_ && modelParts_.size() >= 3) { //[cite: 9]
            modelParts_[1]->transform.rotate.z = idleSin * kIdleArmAngle;  // 左腕[cite: 9]
            modelParts_[2]->transform.rotate.z = -idleSin * kIdleArmAngle; // 右腕[cite: 9]
        }
    }
}

void Player::BehaviorJumpInitialize()
{
    isJumping_ = true; //[cite: 9]
    jumpTimer_ = 0.0f; //[cite: 9]
    jumpVelocityY_ = jumpInitialVelocity_; //[cite: 9]

    modelBody_->transform.scale = { //[cite: 9]
        1.0f + jumpSquashAmount_, //[cite: 9]
        1.0f - jumpSquashAmount_, //[cite: 9]
        1.0f + jumpSquashAmount_ //[cite: 9]
    };
}

void Player::BehaviorJumpUpdate()
{
    if (!isJumping_) return; //[cite: 9]

    jumpTimer_ += 1.0f; //[cite: 9]

    transformBase_.translate.y += jumpVelocityY_; //[cite: 9]
    jumpVelocityY_ -= gravity_; //[cite: 9]

    if (transformBase_.translate.y <= jumpGroundY_) { //[cite: 9]
        transformBase_.translate.y = jumpGroundY_; //[cite: 9]
        jumpVelocityY_ = 0.0f; //[cite: 9]

        modelBody_->transform.scale.x = EMath::Lerp(modelBody_->transform.scale.x, 1.0f + jumpSquashAmount_, 0.3f); //[cite: 9]
        modelBody_->transform.scale.y = EMath::Lerp(modelBody_->transform.scale.y, 1.0f - jumpSquashAmount_, 0.3f); //[cite: 9]
        modelBody_->transform.scale.z = EMath::Lerp(modelBody_->transform.scale.z, 1.0f + jumpSquashAmount_, 0.3f); //[cite: 9]

        for (size_t i = 1; i < modelParts_.size(); ++i) { //[cite: 9]
            modelParts_[i]->transform.rotate.x = EMath::Lerp(modelParts_[i]->transform.rotate.x, 0.0f, 0.3f); //[cite: 9]
            modelParts_[i]->transform.rotate.z = EMath::Lerp(modelParts_[i]->transform.rotate.z, 0.0f, 0.3f); //[cite: 9]
        }

        if (jumpTimer_ > 12.0f) { //[cite: 9]
            modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f }; //[cite: 9]
            isJumping_ = false; //[cite: 9]
        }
    }
    else { //[cite: 9]
        modelBody_->transform.scale.x = EMath::Lerp(modelBody_->transform.scale.x, 1.0f - (jumpSquashAmount_ * 0.5f), 0.1f); //[cite: 9]
        modelBody_->transform.scale.y = EMath::Lerp(modelBody_->transform.scale.y, 1.0f + (jumpSquashAmount_ * 0.5f), 0.1f); //[cite: 9]
        modelBody_->transform.scale.z = EMath::Lerp(modelBody_->transform.scale.z, 1.0f - (jumpSquashAmount_ * 0.5f), 0.1f); //[cite: 9]

        if (jumpVelocityY_ > 0.0f) { //[cite: 9]
            if (modelParts_.size() >= 5) {
                modelParts_[1]->transform.rotate.z = EMath::Lerp(modelParts_[1]->transform.rotate.z, 0.6f, 0.2f);  //[cite: 9]
                modelParts_[2]->transform.rotate.z = EMath::Lerp(modelParts_[2]->transform.rotate.z, -0.6f, 0.2f); //[cite: 9]
                modelParts_[3]->transform.rotate.x = EMath::Lerp(modelParts_[3]->transform.rotate.x, -0.4f, 0.2f); //[cite: 9]
                modelParts_[4]->transform.rotate.x = EMath::Lerp(modelParts_[4]->transform.rotate.x, -0.4f, 0.2f); //[cite: 9]
            }
        }
        else { //[cite: 9]
            if (modelParts_.size() >= 5) {
                modelParts_[1]->transform.rotate.x = EMath::Lerp(modelParts_[1]->transform.rotate.x, 0.3f, 0.2f); //[cite: 9]
                modelParts_[2]->transform.rotate.x = EMath::Lerp(modelParts_[2]->transform.rotate.x, 0.3f, 0.2f); //[cite: 9]
                modelParts_[3]->transform.rotate.x = EMath::Lerp(modelParts_[3]->transform.rotate.x, 0.5f, 0.2f); //[cite: 9]
                modelParts_[4]->transform.rotate.x = EMath::Lerp(modelParts_[4]->transform.rotate.x, 0.5f, 0.2f); //[cite: 9]
            }
        }
    }
}