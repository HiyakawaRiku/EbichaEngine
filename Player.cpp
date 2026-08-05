#include "Player.h"
#include <cmath>


void Player::Initialize()
{
	transformBase_.Initialize();
	transformBase_.translate = { 0, 10, 0 };

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

	UpdateFloatingGimmick();

	Vector3 velocity_ = {};

	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)|| Input::GetInstance()->PushKey(DIK_W) || Input::GetInstance()->PushKey(DIK_S)) {
		Vector3 acceleration{};
		if (Input::GetInstance()->PushKey(DIK_D)) {
			acceleration.x += kAcceleration;
		}
		else if (Input::GetInstance()->PushKey(DIK_A)) {
			acceleration.x -= kAcceleration;
		}
		else if (Input::GetInstance()->PushKey(DIK_W)) {
			acceleration.z += kAcceleration;
		}
		else if (Input::GetInstance()->PushKey(DIK_S)) {
			acceleration.z -= kAcceleration;
		}

		velocity_ += acceleration;
	}

	//// 十字キー右（長押し移動など）
	//if (Input::GetInstance()->PushButton(XINPUT_GAMEPAD_DPAD_RIGHT)) {
	//	// MoveRight()
	//}

	// 左スティックの傾きで移動
	float moveX = Input::GetInstance()->GetLeftStickX(); // -1.0 〜 1.0
	float moveZ = Input::GetInstance()->GetLeftStickY();

	// 入力によるローカル移動ベクトル（画面の手前/奥、左右）
	Vector3 localMove = { moveX + velocity_.x, 0.0f, moveZ + velocity_.z };

	// 入力がある場合のみ処理
	if (localMove.x != 0.0f || localMove.z != 0.0f) {
		Vector3 worldMove = localMove;

		// カメラ情報が存在する場合、カメラのY軸回転に合わせて移動ベクトルを回転
		if (activeCamera_) {
			float cameraRotateY = activeCamera_->transform_.rotate.y;
			Matrix4x4 matRotateY = MakeRotateYMatrix(cameraRotateY);
			worldMove = TransformNormal(localMove, matRotateY);
		}

		// 1. 目標角度を算出
		float targetAngle = std::atan2(worldMove.x, worldMove.z);

		// 2. EMath::LerpShortAngle を使って現在角度から目標角度へ滑らかに補間
		modelBody_->transform.rotate.y = EMath::LerpShortAngle(
			modelBody_->transform.rotate.y,
			targetAngle,
			kRotateSpeed
		);

		// 移動量を位置に加算
		transformBase_.translate += worldMove;
	}

	//modelBody_->transform.UpdateMatrix();
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
