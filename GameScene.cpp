#include "GameScene.h"

void GameScene::Initialize() {
	// シングルトンインスタンスの取得
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// オーディオの初期化
	audio_->Initialize();

	// オブジェクトの生成と初期化
	sprite_ = new Sprite();
	sprite_->Initialize();

	//sphere_ = new Sphere();
	//sphere_->Initialize();

	//modelTeapot_ = new Model();
	//modelTeapot_->Initialize("teapot.obj");

	//modelBunny_ = new Model();
	//modelBunny_->Initialize("bunny.obj");

	player_ = new Player();
	player_->Initialize();

	skydome_ = new Skydome();
	skydome_->Initialize();

	ground_ = new Ground();
	ground_->Initialize();

	followCamera_ = new FollowCamera();
	followCamera_->Initialize();
	followCamera_->SetTarget(&player_->GetTransform());
	player_->SetViewProjection(&followCamera_->GetCamera());

	//modelMultiMesh_ = new Model();
	//modelMultiMesh_->Initialize("resources", "multiMesh.obj");

	// カメラのセットアップ
	normalCamera_ = std::make_unique<Camera>();
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	activeCamera_ = normalCamera_.get();

	// テクスチャの初期化
	dxCommon_->InitializeTexture("resources/uvChecker.png", 1);
	dxCommon_->InitializeTexture("resources/monsterBall.png", 2);
	dxCommon_->InitializeTexture("resources/sky_sphere.png", 3);
	dxCommon_->InitializeTexture("resources/ground_leaf.png", 4);
	dxCommon_->InitializeTexture("resources/fence.png", 5);
	dxCommon_->InitializeTexture("resources/EModel.png", 6);
	dxCommon_->InitializeTexture("resources/player.png", 7);

	// 音声ファイルのロード & 再生開始
	seHandle_ = audio_->LoadAudioSource("Resources/Alarm01.wav");
	bgmHandle_ = audio_->LoadAudioSource("Resources/420_long_BPM108.mp3");
	//bgmHandle_ = audio_->LoadAudioSource("Resources/koi.mp3");
	audio_->PlayWave(bgmHandle_, true, 0.5f);
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

	// オブジェクト更新
	sprite_->Update(activeCamera_);
	//sphere_->Update(activeCamera_);
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

	// UV Transform の更新計算
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite_.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite_.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite_.translate));
	sprite_->materialData->uvTransform = uvTransformMatrix;
}

void GameScene::Draw() {
	// 描画前処理
	dxCommon_->PreDraw();

#ifdef _DEBUG
	ImGui::Begin("Settings");

	ImGui::Checkbox("Change Camera", &useDebugCamera_);

	ImGui::End();
#endif

	// 各オブジェクトの描画
	//sprite_->Draw(1);
	//sphere_->Draw(1);
	//modelTeapot_->Draw(1);
	//modelBunny_->Draw(1);
	//modelMultiMesh_->Draw(1);


	skydome_->Draw();
	ground_->Draw();
	player_->Draw();

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

	// 動的割当オブジェクトの破棄
	delete sprite_;
	sprite_ = nullptr;

	//delete sphere_;
	//sphere_ = nullptr;

	//delete modelTeapot_;
	//modelTeapot_ = nullptr;

	//delete modelBunny_;
	//modelBunny_ = nullptr;

	//delete modelMultiMesh_;
	//modelMultiMesh_ = nullptr;
}