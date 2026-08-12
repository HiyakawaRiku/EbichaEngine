#include "GameScene.h"

void GameScene::Initialize() {
	// シングルトンインスタンスの取得
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// TextureManager の初期化 (DirectXCommon の初期化後に行う)
	TextureManager::GetInstance()->Initialize(dxCommon_->GetDevice(), dxCommon_->GetSrvHeap());

	// オーディオの初期化
	audio_->Initialize();

	//sprite_ = std::make_unique<Sprite>();
	//sprite_->Initialize();
	//sprite_->transform.translate = { 100.0f, 50.0f, 0.0f };

	sphere_ = std::make_unique<Sphere>();
	sphere_->Initialize();

	player_ = new Player();
	player_->Initialize();

	skydome_ = new Skydome();
	skydome_->Initialize();

	ground_ = new Ground();
	ground_->Initialize();

	particle_ = new Particle();
	particle_->Initialize();

	followCamera_ = new FollowCamera();
	followCamera_->Initialize();
	followCamera_->SetTarget(&player_->GetTransform());
	player_->SetViewProjection(&followCamera_->GetCamera());

	// カメラのセットアップ
	normalCamera_ = std::make_unique<Camera>();
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	activeCamera_ = normalCamera_.get();

	// 音声ファイルのロード & 再生開始
	seHandle_ = audio_->LoadAudioSource("Resources/Alarm01.wav");
	bgmHandle_ = audio_->LoadAudioSource("Resources/420_long_BPM108.mp3");
	//bgmHandle_ = audio_->LoadAudioSource("Resources/koi.mp3");
	audio_->PlayWave(bgmHandle_, true, 0.5f);
	dxCommon_->SetBlendMode(blendMode_);

	textureHandle_ = TextureManager::GetInstance()->Load("resources/monsterBall.png", dxCommon_->GetCommandList());
	
}

void GameScene::Update() {
	// 入力更新
	input_->Update();

	if (useDebugCamera_) {
		activeCamera_ = debugCamera_.get();
	}
	else {
		//activeCamera_ = normalCamera_.get();
		activeCamera_=&followCamera_->GetCamera();
	}

	// カメラ更新
	followCamera_->Update();
	normalCamera_->Update();
	if (activeCamera_ != normalCamera_.get()) {
		activeCamera_->Update();
	}
	player_->Update(activeCamera_);
	skydome_->Update(activeCamera_);
	ground_->Update(activeCamera_);
	particle_->Update(activeCamera_);

	// オブジェクト更新
	//sprite_->Update(activeCamera_);
	sphere_->Update(activeCamera_);
	//modelTeapot_->Update(activeCamera_);
	//modelBunny_->Update(activeCamera_);
	//modelMultiMesh_->Update(activeCamera_);

	// デバッググリッド設定
	DebugRenderer::AddGrid(100.0f, 10, { 0.5f, 0.5f, 0.5f, 1.0f });



	// キー操作による処理
	if (input_->PushKey(DIK_P)) {
		activeCamera_->transform_.rotate.x -= 0.03f;
	}
	if (input_->PushButton(XINPUT_GAMEPAD_DPAD_RIGHT)) {
		activeCamera_->transform_.rotate.x += 0.03f;
	}

	if (useDebugCamera_) {
		debugCamera_->DrawFrustum(normalCamera_.get());
	}

	//// UV Transform の更新計算
	//Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite_.scale);
	//uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite_.rotate.z));
	//uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite_.translate));
	//sprite_->GetMaterialData()->uvTransform = uvTransformMatrix;
}

void GameScene::Draw() {

	dxCommon_->SetPipelineType(PipelineType::kObject3D);
	// 描画前処理
	dxCommon_->PreDraw();

#ifdef _DEBUG
	ImGui::Begin("Settings");

	ImGui::Checkbox("Change Camera", &useDebugCamera_);
	ImGui::DragFloat3("position", &sphere_->transform.translate.x, 0.1f);
	ImGui::DragFloat3("position2", &sphere_->transform.scale.x, 0.1f);

	// ライティングタイプの選択 (0: None, 1: Lambert, 2: Half-Lambert)
	const char* lightingTypes[] = { "Unlit (0)", "Lambert (1)", "Half-Lambert (2)" };
	int currentLighting = static_cast<int>(sphere_->lightingType);
	if (ImGui::Combo("Lighting Type", &currentLighting, lightingTypes, IM_ARRAYSIZE(lightingTypes))) {
		sphere_->lightingType = static_cast<uint32_t>(currentLighting);
	}

	int currentIndex = static_cast<int>(blendMode_);
	if (ImGui::Combo("Blend Mode", &currentIndex,blendModeNames_)) {
		blendMode_ = static_cast<BlendMode>(currentIndex);
		DirectXCommon::GetInstance()->SetBlendMode(blendMode_);
	};

	// =========================================================
	// ポイントライト調整用 UI
	// =========================================================
	if (PointLight* pointLight = sphere_->GetPointLightData()) {
		if (ImGui::TreeNode("Point Light")) {
			ImGui::ColorEdit4("Color", &pointLight->color.x);
			ImGui::DragFloat3("Position", &pointLight->position.x, 0.1f);
			ImGui::DragFloat("Intensity", &pointLight->intensity, 0.05f, 0.0f, 100.0f);
			ImGui::DragFloat("Radius", &pointLight->radius, 0.1f, 0.1f, 1000.0f);
			ImGui::DragFloat("Decay", &pointLight->decay, 0.05f, 0.0f, 10.0f);
			ImGui::TreePop();
		}
	}

	// =========================================================
	// スポットライト調整用 UI
	// =========================================================
	if (SpotLight* spotLight = sphere_->GetSpotLightData()) {
		if (ImGui::TreeNode("Spot Light")) {
			ImGui::ColorEdit4("Color", &spotLight->color.x);
			ImGui::DragFloat3("Position", &spotLight->position.x, 0.1f);
			ImGui::DragFloat3("Direction", &spotLight->direction.x, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("Intensity", &spotLight->intensity, 0.05f, 0.0f, 100.0f);
			ImGui::DragFloat("Distance", &spotLight->distance, 0.1f, 0.1f, 1000.0f);
			ImGui::DragFloat("Decay", &spotLight->decay, 0.05f, 0.0f, 10.0f);

			// 角度 (cos値から角度(度)へ変換して調整)
			static float angleDeg = 30.0f;
			static float falloffStartDeg = 15.0f;

			if (ImGui::DragFloat("Angle (deg)", &angleDeg, 0.5f, 0.0f, 90.0f)) {
				spotLight->cosAngle = std::cos(angleDeg * 3.14159265f / 180.0f);
			}
			if (ImGui::DragFloat("Falloff Start (deg)", &falloffStartDeg, 0.5f, 0.0f, angleDeg)) {
				spotLight->cosFalloffStart = std::cos(falloffStartDeg * 3.14159265f / 180.0f);
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif

	// 各オブジェクトの描画
	//sprite_->Draw(1);
	sphere_->Draw(textureHandle_);
	//modelTeapot_->Draw(1);
	//modelBunny_->Draw(1);
	//modelMultiMesh_->Draw(1);
	dxCommon_->SetPipeline(PipelineType::kObject3D, blendMode_, DepthWrite::kEnable);

	skydome_->Draw();
	ground_->Draw();
	player_->Draw();

	dxCommon_->SetPipeline(PipelineType::kParticle, BlendMode::kAdd, DepthWrite::kDisable);

	particle_->Draw();

	// デバッグレンダラーの描画適用
	DebugRenderer::Flush(activeCamera_);

	// 描画後処理
	dxCommon_->PostDraw();


}

void GameScene::Finalize() {
	// サウンド解放
	if (audio_) {
		audio_->Unload(bgmHandle_);
		audio_->Unload(seHandle_);
		audio_->Finalize();
	}
}