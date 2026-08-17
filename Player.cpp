#include "Player.h"
#include <cmath>
#include <algorithm>

void Player::Initialize()
{
    // パーツ構成を空にする（Cube単体にする）
    std::vector<BaseCharacter::PartConfig> partConfigs = {};

    // 親クラスの初期化関数に渡す
    BaseCharacter::Initialize("sphere", partConfigs, "resources/white1x1.png");
    transformBase_.translate = { 0.0f, 1.0f, 0.0f };

    hammerPartIndex_ = -1; // ハンマーは使用しない

    InitializeFloatingGimmick();

    // コライダーの半径を設定
    SetColliderRadius(1.0f);

    // HP & 無敵状態の初期化
    hp_ = kMaxHp_;
    isInvincible_ = false;
    invincibleTimer_ = 0.0f;

    // 各変数の初期化
    moveVelocity_ = { 0.0f, 0.0f, 0.0f };
    sphereRollAngleX_ = 0.0f;
    moveDashTimer_ = 0.0f;
    idleMotionTimer_ = 0.0f;
    isLandingBounce_ = false;
    landingBounceTimer_ = 0.0f;
    isDashAttacking_ = false;
}

void Player::Update(Camera* activeCamera)
{
    // 親クラスの更新を実行
    BaseCharacter::Update(activeCamera);

    // 浮遊ギミックの更新処理を呼ぶ
    UpdateFloatingGimmick();

    // 無敵時間のカウントダウン処理
    if (isInvincible_) {
        invincibleTimer_ -= 1.0f;
        if (invincibleTimer_ <= 0.0f) {
            isInvincible_ = false;
            invincibleTimer_ = 0.0f;
        }
    }

    // プレイヤー固有の挙動更新
    BehaviorRootUpdate(activeCamera);

    // ImGui による HP ステータス表示・テストデバッグ用ウィンドウ
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

#ifdef _DEBUG
    ImGui::Begin("Position Monitor");

    // プレイヤーの位置・移動パラメータ・突進状態
    Vector3 playerPos = transformBase_.translate;
    float currentSpeed = std::sqrt(moveVelocity_.x * moveVelocity_.x + moveVelocity_.z * moveVelocity_.z);
    ImGui::Text("Player Pos   : X:%.2f, Y:%.2f, Z:%.2f", playerPos.x, playerPos.y, playerPos.z);
    ImGui::Text("Current Speed: %.3f", currentSpeed);
    ImGui::Text("Dash Timer   : %.1f / %.1f", moveDashTimer_, kDashAccelTime);
    ImGui::Text("Dash Attack  : %s", isDashAttacking_ ? "ACTIVE (MAX SPEED)" : "Inactive");

    ImGui::End();
#endif
}

// 被弾ダメージ処理
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
}

AABB Player::GetColliderAABB() const
{
    Vector3 center = transformBase_.translate; // プレイヤーの現在位置

    AABB aabb;
    aabb.min = center - boxExtents_;
    aabb.max = center + boxExtents_;
    return aabb;
}

void Player::InitializeFloatingGimmick()
{
    floatingParameter_ = 0.0f;
}

void Player::UpdateFloatingGimmick()
{
    const uint16_t cycle = (uint16_t)frame_;
    const float step = 2.0f * 3.1415926f / cycle;

    floatingParameter_ += step;
    floatingParameter_ = std::fmod(floatingParameter_, 2.0f * 3.1415926f);

    modelBody_->transform.translate.y = std::sin(floatingParameter_) * floatingAmplitude;

#ifdef _DEBUG
    ImGui::Begin("Player");
    ImGui::SliderFloat("amplitude", &floatingAmplitude, 0.1f, 1.0f);
    ImGui::End();
#endif
}

void Player::BehaviorRootUpdate(Camera* activeCamera_)
{
    // -------------------------------------------------------------
    // 1. ジャンプ入力 & 処理
    // -------------------------------------------------------------
    if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isJumping_) {
        BehaviorJumpInitialize();
    }

    if (isJumping_ && jumpVelocityY_ > 0.0f) {
        if (Input::GetInstance()->ReleaseKey(DIK_SPACE)) {
            jumpVelocityY_ *= jumpCutMultiplier_;
        }
    }

    if (isJumping_) {
        BehaviorJumpUpdate();
    }

    // -------------------------------------------------------------
    // 2. 移動（振り向き慣性 & 立ち止まり慣性）
    // -------------------------------------------------------------
    Vector3 inputDir = {};
    if (Input::GetInstance()->PushKey(DIK_D)) { inputDir.x += 1.0f; }
    if (Input::GetInstance()->PushKey(DIK_A)) { inputDir.x -= 1.0f; }
    if (Input::GetInstance()->PushKey(DIK_W)) { inputDir.z += 1.0f; }
    if (Input::GetInstance()->PushKey(DIK_S)) { inputDir.z -= 1.0f; }

    float stickX = Input::GetInstance()->GetLeftStickX();
    float stickZ = Input::GetInstance()->GetLeftStickY();
    inputDir.x += stickX;
    inputDir.z += stickZ;

    bool isInputting = (inputDir.x != 0.0f || inputDir.z != 0.0f);

    if (isInputting) {
        // 入力ベクトルの正規化
        float length = std::sqrt(inputDir.x * inputDir.x + inputDir.z * inputDir.z);
        if (length > 0.001f) {
            inputDir.x /= length;
            inputDir.z /= length;
        }

        // カメラ視点に応じたベクトル変換
        if (activeCamera_) {
            float cameraRotateY = activeCamera_->transform_.rotate.y;
            Matrix4x4 matRotateY = MakeRotateYMatrix(cameraRotateY);
            inputDir = TransformNormal(inputDir, matRotateY);
        }

        // ダッシュタイマー加算（緩やかな加速計算用）
        moveDashTimer_ += 1.0f;
        float dashRatio = (std::min)(moveDashTimer_ / kDashAccelTime, 1.0f);
        float smoothedRatio = dashRatio * dashRatio;
        float targetSpeed = EMath::Lerp(kBaseMoveSpeed, kMaxMoveSpeed, smoothedRatio);

        // ★ 1. 新しい入力方向と「現在の進行方向」の内積（向きの関係）を計算
        float currentSpeed = std::sqrt(moveVelocity_.x * moveVelocity_.x + moveVelocity_.z * moveVelocity_.z);
        if (currentSpeed > 0.001f) {
            Vector3 currentDir = { moveVelocity_.x / currentSpeed, 0.0f, moveVelocity_.z / currentSpeed };
            float dot = currentDir.x * inputDir.x + currentDir.z * inputDir.z;

            // ★ 2. 真逆または急角度への切り返し（内積が負）の場合、スピードとダッシュタイマーを即座にリセット
            if (dot < 0.0f) {
                moveVelocity_ = { 0.0f, 0.0f, 0.0f };
                moveDashTimer_ = 0.0f; // 切り返し時にダッシュタイマーもリセットしたい場合
            }
        }

        // ★ 3. 速度ベクトルを新しい入力方向に直接設定（滑る補正を削除）
        moveVelocity_.x = inputDir.x * targetSpeed;
        moveVelocity_.z = inputDir.z * targetSpeed;

        // 最高速度に達している場合は突進攻撃判定フラグを立てる
        isDashAttacking_ = (dashRatio >= 1.0f);
    }
    else {
        // ★【立ち止まりの慣性】キーを離した時は摩擦で滑りながら停止
        moveVelocity_.x *= kStopFriction;
        moveVelocity_.z *= kStopFriction;

        // タイマー減衰・突進解除
        moveDashTimer_ = (std::max)(0.0f, moveDashTimer_ - 2.0f);
        isDashAttacking_ = false;
    }

    // 極小値のクランプ（振れ防止）
    if (std::abs(moveVelocity_.x) < 0.0001f) moveVelocity_.x = 0.0f;
    if (std::abs(moveVelocity_.z) < 0.0001f) moveVelocity_.z = 0.0f;

    float currentSpeed = std::sqrt(moveVelocity_.x * moveVelocity_.x + moveVelocity_.z * moveVelocity_.z);
    bool isMoving = (currentSpeed > 0.001f);

    if (isMoving) {
        // 1. 進行方向（Y軸旋回）への回転
        float targetAngle = std::atan2(moveVelocity_.x, moveVelocity_.z);
        modelBody_->transform.rotate.y = EMath::LerpShortAngle(
            modelBody_->transform.rotate.y,
            targetAngle,
            kRotateSpeed
        );

        // 2. 転がり回転 (ジャンプ中でない場合)
        if (!isJumping_) {
            float deltaAngle = currentSpeed / sphereRadius_;
            sphereRollAngleX_ += deltaAngle;
            modelBody_->transform.rotate.x = sphereRollAngleX_;

            // 突進攻撃中は球体を前後に変形してスピード感を出す
            if (isDashAttacking_) {
                modelBody_->transform.scale = { 0.9f, 0.9f, 1.25f };
            }
            else if (!isLandingBounce_) {
                modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f };
            }
        }

        // 位置の更新
        transformBase_.translate += moveVelocity_;

        // =========================================================
        // ★ 原点（X:0, Z:0）から半径50以内に制限する処理
        // =========================================================
        const float kMaxRadius = 100.0f; // 制限半径

        // 原点からのXZ平面上の距離を計算
        float distance = std::sqrt(transformBase_.translate.x * transformBase_.translate.x +
            transformBase_.translate.z * transformBase_.translate.z);

        // 距離が50を超えていたら境界上に押し戻す
        if (distance > kMaxRadius) {
            // 原点からの正規化方向ベクトルを掛けて、距離をちょうど50に収める
            transformBase_.translate.x = (transformBase_.translate.x / distance) * kMaxRadius;
            transformBase_.translate.z = (transformBase_.translate.z / distance) * kMaxRadius;

            // 境界に引っかかった際、外向きの速度（進行方向の力）をクリアして突っかかりを滑らかにする
            moveVelocity_ = { 0.0f, 0.0f, 0.0f };
        }
    }
    else {
        // -------------------------------------------------------------
        // 3. 球体の待機モーション (呼吸・ゆらゆら動き)
        // -------------------------------------------------------------
        if (!isJumping_) {
            idleMotionTimer_ += kIdleMotionSpeed;
            float idleSin = std::sin(idleMotionTimer_);
            float idleCos = std::cos(idleMotionTimer_ * 0.5f);

            // 呼吸に伴う伸縮（Squash & Stretch）
            if (!isLandingBounce_) {
                float scaleY = 1.0f + idleSin * 0.05f;
                float scaleXZ = 1.0f - idleSin * 0.025f;
                modelBody_->transform.scale = { scaleXZ, scaleY, scaleXZ };
            }

            // 微小な上下左右揺れ
            modelBody_->transform.translate.y = idleSin * 0.08f;
            modelBody_->transform.translate.x = idleCos * 0.03f;
        }
    }

    // -------------------------------------------------------------
    // 4. 着地バウンド（Squash復元）のアニメーション更新
    // -------------------------------------------------------------
    if (isLandingBounce_ && modelBody_) {
        landingBounceTimer_ += 0.15f;
        float bounceSin = std::sin(landingBounceTimer_ * 3.1415926f);

        float scaleY = 1.0f - bounceSin * 0.25f;
        float scaleXZ = 1.0f + bounceSin * 0.15f;
        modelBody_->transform.scale = { scaleXZ, scaleY, scaleXZ };

        if (landingBounceTimer_ >= 1.0f) {
            isLandingBounce_ = false;
            landingBounceTimer_ = 0.0f;
            modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f };
        }
    }
}

void Player::BehaviorJumpInitialize()
{
    isJumping_ = true;
    jumpVelocityY_ = jumpInitialVelocity_;
    isLandingBounce_ = false;

    if (modelBody_) {
        jumpStartRotX_ = modelBody_->transform.rotate.x;
        jumpTargetRotX_ = jumpStartRotX_ + (3.1415926f * 2.0f); // 空中での1回転
    }
}

void Player::BehaviorJumpUpdate()
{
    if (!isJumping_) return;

    // Y軸位置更新・重力演算
    transformBase_.translate.y += jumpVelocityY_;
    if (jumpVelocityY_ < 0.0f) {
        jumpVelocityY_ -= gravity_ * fallGravityMultiplier_;
    }
    else {
        jumpVelocityY_ -= gravity_;
    }

    // 空中での前転と変形演出
    if (modelBody_) {
        modelBody_->transform.rotate.x = EMath::Lerp(
            modelBody_->transform.rotate.x,
            jumpTargetRotX_,
            0.15f
        );

        if (jumpVelocityY_ > 0.0f) {
            modelBody_->transform.scale = { 0.85f, 1.20f, 0.85f };
        }
        else {
            modelBody_->transform.scale = { 1.15f, 0.85f, 1.15f };
        }
    }

    // 着地判定
    if (transformBase_.translate.y <= jumpGroundY_) {
        transformBase_.translate.y = jumpGroundY_;
        jumpVelocityY_ = 0.0f;
        isJumping_ = false;

        // 着地バウンドアニメーションの開始
        isLandingBounce_ = true;
        landingBounceTimer_ = 0.0f;

        if (modelBody_) {
            sphereRollAngleX_ = std::fmod(modelBody_->transform.rotate.x, 3.1415926f * 2.0f);
        }
    }
}

// 突進攻撃の判定（OBB）を取得する関数
OBB Player::GetDashAttackOBB() const
{
    OBB obb;
    obb.extents = dashAttackExtents_;

    if (!isDashAttacking_) {
        obb.center = { 0.0f, -999.0f, 0.0f }; // 無効時
        return obb;
    }

    Matrix4x4 matWorld = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, modelBody_->transform.rotate, transformBase_.translate);
    obb.transform = matWorld;
    obb.center = transformBase_.translate;

    return obb;
}