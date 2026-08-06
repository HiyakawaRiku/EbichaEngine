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
		{ "legL1.obj", {  -0.2f, -0.5f, 0.0f } },
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

void Player::Update(Camera* activeCamera_)
{
	//UpdateFloatingGimmick();

	BehaviorRootUpdate(activeCamera_);

	transformBase_.UpdateMatrix();

	modelBody_->Update(activeCamera_);
	for (auto& part : modelParts_) {

		part->Update(activeCamera_);
	}
}

void Player::Draw()
{
	modelBody_->Draw(textureHandle_);
	for (auto& part : modelParts_) {
		part->Draw(textureHandle_);
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
			modelParts_[1]->transform.rotate.x = swing; // 左腕[cite: 1]
			modelParts_[2]->transform.rotate.x = -swing; // 右腕[cite: 1]
		}
		modelParts_[3]->transform.rotate.x = -swing; // 左脚[cite: 1]
		modelParts_[4]->transform.rotate.x = swing; // 右脚[cite: 1]

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
			modelParts_[1]->transform.rotate.z = idleSin * kIdleArmAngle; // 左腕の揺れ
			modelParts_[2]->transform.rotate.z = -idleSin * kIdleArmAngle; // 右腕の揺れ
		}
	}
}
