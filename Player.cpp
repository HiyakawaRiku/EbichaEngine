#include "Player.h"
#include <cmath>

void Player::Initialize()
{
	transformBase_.Initialize();
	transformBase_.translate = { 0, 1.5f, 0 };

	modelBody_ = std::make_unique<Model>();
	modelBody_->Initialize("Body.obj");
	modelBody_->transform.translate = { 0.0f, 0.0f, 0.0f };
	modelBody_->transform.parent = &transformBase_;

	struct PartConfig {
		std::string filename;
		Vector3 position;
	};

	const PartConfig partConfigs[] = {
		{ "Head.obj",  {  0.0f, 0.5f, 0.0f } },
		{ "armL1.obj", { -0.5f, 0.5f, 0.0f } },
		{ "armR1.obj", {  0.5f, 0.5f, 0.0f } },
		{ "legL1.obj", { -0.2f, -0.5f, 0.0f } },
		{ "legR1.obj", {  0.2f, -0.5f, 0.0f } },
	};

	modelParts_.clear();
	for (const auto& config : partConfigs) {
		auto part = std::make_unique<Model>();
		part->Initialize(config.filename);
		part->transform.translate = config.position;
		part->transform.parent = &modelBody_->transform;

		modelParts_.push_back(std::move(part));
	}

	textureHandle_ = TextureManager::GetInstance()->Load("resources/tex.png", DirectXCommon::GetInstance()->GetCommandList());

	InitializeFloatingGimmick();
}

void Player::Update(Camera* activeCamera)
{
	// カメラ参照の更新保持
	viewProjection_ = activeCamera;
	modelBody_->Update(activeCamera);
	for (auto& part : modelParts_) {
		part->Update(activeCamera);
	}
	BehaviorRootUpdate(activeCamera);

	transformBase_.UpdateMatrix();


	// ImGui によるパラメータ調整ウィンドウ
	ImGui::Begin("Player Jump Settings");
	ImGui::SliderFloat("Initial Velocity", &jumpInitialVelocity_, 0.1f, 1.5f);
	ImGui::SliderFloat("Gravity", &gravity_, 0.005f, 0.1f);
	ImGui::SliderFloat("Squash Amount", &jumpSquashAmount_, 0.0f, 0.6f);
	ImGui::SliderFloat("Ground Y", &jumpGroundY_, 0.0f, 5.0f);

	// ImGui上から直接テストジャンプをトリガーできるボタン
	if (ImGui::Button("Test Jump") && !isJumping_) {
		BehaviorJumpInitialize();
	}
	ImGui::End();
}

void Player::Draw()
{
	Camera* camera = viewProjection_;

	// 1. まずルート（全体の位置）の行列を更新
	transformBase_.UpdateMatrix();

	// 2. 親である胴体（modelBody_）の行列を更新＆描画
	if (modelBody_) {
		// 胴体の transform.parent (&transformBase_) を考慮して行列を更新
		modelBody_->transform.UpdateMatrix();
		modelBody_->Draw(viewProjection_, textureHandle_);
	}

	// 3. 子であるパーツ（modelParts_）の行列を更新＆描画
	// ★親（modelBody_）の matWorld が確定した後にパーツを描画するのがポイント★
	for (auto& part : modelParts_) {
		if (part) {
			// 親（modelBody_->transform）の最新の matWorld を使って自身の行列を更新
			part->transform.UpdateMatrix();
			part->Draw(viewProjection_, textureHandle_);
		}
	}
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
	// BehaviorRootUpdate 内で入力検知
	if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isJumping_) {
		BehaviorJumpInitialize();
	}

	// ジャンプ中であれば更新処理を呼ぶ
	if (isJumping_) {
		BehaviorJumpUpdate();
	}

	// ----------------------------------------------------
	// 1. 移動入力の取得
	// ----------------------------------------------------
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

	// ----------------------------------------------------
	// 4. 移動 & 歩行アニメーション処理
	// ----------------------------------------------------
	bool isMoving = (localMove.x != 0.0f || localMove.z != 0.0f);

	if (isMoving) {
		walkTimer_ += kWalkSpeed;
		float swing = std::sin(walkTimer_) * kWalkAngle;

		if (!isAttacking_) {
			modelParts_[1]->transform.rotate.x = swing;  // 左腕[cite: 7]
			modelParts_[2]->transform.rotate.x = -swing; // 右腕[cite: 7]
		}
		modelParts_[3]->transform.rotate.x = -swing; // 左脚[cite: 7]
		modelParts_[4]->transform.rotate.x = swing;  // 右脚[cite: 7]

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

		// 待機タイマーの更新
		idleTimer_ += kIdleSpeed;
		float idleSin = std::sin(idleTimer_);

		// 1. 体全体（modelBody_）を少し上下させて息づかいを表現
		modelBody_->transform.translate.y = idleSin * kIdleBreathing;

		// 2. 手足を初期姿勢に補間しつつ、腕をわずかに前後・開閉させる
		for (int i = 1; i <= 4; ++i) {
			if ((i == 1 || i == 2) && isAttacking_) continue;

			// 基本位置に補間 ( Lerp )
			modelParts_[i]->transform.rotate.x = EMath::Lerp(modelParts_[i]->transform.rotate.x, 0.0f, 0.2f);
		}

		// 3. 待機中の腕の微振動（呼吸に合わせて開閉・前後に揺らす）
		if (!isAttacking_) {
			modelParts_[1]->transform.rotate.z = idleSin * kIdleArmAngle;  // 左腕の揺れ
			modelParts_[2]->transform.rotate.z = -idleSin * kIdleArmAngle; // 右腕の揺れ
		}
	}
}

void Player::BehaviorJumpInitialize()
{
	isJumping_ = true;
	jumpTimer_ = 0.0f;
	jumpVelocityY_ = jumpInitialVelocity_; // 初速を設定

	// 予備動作（踏み込み）: 設定したつぶれ具合に応じてスケールを変更
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

	// 1. 物理移動（Y座標の更新）
	transformBase_.translate.y += jumpVelocityY_;
	jumpVelocityY_ -= gravity_; // 重力適用

	// 2. 着地判定 & アニメーション
	if (transformBase_.translate.y <= jumpGroundY_) {
		transformBase_.translate.y = jumpGroundY_;
		jumpVelocityY_ = 0.0f;

		// --- 着地フェーズ ---
		// 衝撃による潰れ（`jumpSquashAmount_`分つぶす）
		modelBody_->transform.scale.x = EMath::Lerp(modelBody_->transform.scale.x, 1.0f + jumpSquashAmount_, 0.3f);
		modelBody_->transform.scale.y = EMath::Lerp(modelBody_->transform.scale.y, 1.0f - jumpSquashAmount_, 0.3f);
		modelBody_->transform.scale.z = EMath::Lerp(modelBody_->transform.scale.z, 1.0f + jumpSquashAmount_, 0.3f);

		// 手足をまっすぐ戻す
		for (int i = 1; i <= 4; ++i) {
			modelParts_[i]->transform.rotate.x = EMath::Lerp(modelParts_[i]->transform.rotate.x, 0.0f, 0.3f);
			modelParts_[i]->transform.rotate.z = EMath::Lerp(modelParts_[i]->transform.rotate.z, 0.0f, 0.3f);
		}

		// しばらくしたら立ち姿勢のスケールに戻してジャンプ終了
		if (jumpTimer_ > 12.0f) {
			modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f };
			isJumping_ = false;
		}
	}
	else {
		// --- 空中フェーズ ---
		// 潰れた体を空中で細長く伸ばす
		modelBody_->transform.scale.x = EMath::Lerp(modelBody_->transform.scale.x, 1.0f - (jumpSquashAmount_ * 0.5f), 0.1f);
		modelBody_->transform.scale.y = EMath::Lerp(modelBody_->transform.scale.y, 1.0f + (jumpSquashAmount_ * 0.5f), 0.1f);
		modelBody_->transform.scale.z = EMath::Lerp(modelBody_->transform.scale.z, 1.0f - (jumpSquashAmount_ * 0.5f), 0.1f);

		if (jumpVelocityY_ > 0.0f) {
			// 上昇中: 腕を広げ、脚を引く
			modelParts_[1]->transform.rotate.z = EMath::Lerp(modelParts_[1]->transform.rotate.z, 0.6f, 0.2f);  // 左腕
			modelParts_[2]->transform.rotate.z = EMath::Lerp(modelParts_[2]->transform.rotate.z, -0.6f, 0.2f); // 右腕
			modelParts_[3]->transform.rotate.x = EMath::Lerp(modelParts_[3]->transform.rotate.x, -0.4f, 0.2f); // 左脚
			modelParts_[4]->transform.rotate.x = EMath::Lerp(modelParts_[4]->transform.rotate.x, -0.4f, 0.2f); // 右脚
		}
		else {
			// 落下中: 着地準備姿勢
			modelParts_[1]->transform.rotate.x = EMath::Lerp(modelParts_[1]->transform.rotate.x, 0.3f, 0.2f);
			modelParts_[2]->transform.rotate.x = EMath::Lerp(modelParts_[2]->transform.rotate.x, 0.3f, 0.2f);
			modelParts_[3]->transform.rotate.x = EMath::Lerp(modelParts_[3]->transform.rotate.x, 0.5f, 0.2f);
			modelParts_[4]->transform.rotate.x = EMath::Lerp(modelParts_[4]->transform.rotate.x, 0.5f, 0.2f);
		}
	}
}