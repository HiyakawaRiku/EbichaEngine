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

	int currentIndex = static_cast<int>(blendMode_);
	if (ImGui::Combo("Blend Mode", &currentIndex,blendModeNames_)) {
		blendMode_ = static_cast<BlendMode>(currentIndex);
		DirectXCommon::GetInstance()->SetBlendMode(blendMode_);
	};

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