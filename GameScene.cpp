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

	// =========================================================
	// キャラクターの生成と一括管理コンテナへの追加
	// =========================================================
	characters_.clear();

	// 1. プレイヤーの生成・初期化
	auto player = std::make_unique<Player>();
	player->Initialize();
	player_ = player.get(); // FollowCamera 設定用のポインタを保持
	characters_.push_back(std::move(player));

	// 2. 敵生成
	auto enemy = std::make_unique<Enemy>();
	enemy->Initialize();
	enemy->SetTargetPlayer(player_); // ★ 敵にプレイヤーのポインタを渡す
	enemy_ = enemy.get();            // ★ 参照保持用（GameScene.hに Enemy* enemy_ = nullptr; を宣言）
	characters_.push_back(std::move(enemy));

	// =========================================================
	// 背景・カメラ等の初期化
	// =========================================================
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	particle_ = std::make_unique<Particle>();
	particle_->Initialize();

	followCamera_ = std::make_unique<FollowCamera>();
	followCamera_->Initialize();
	followCamera_->SetTarget(&player_->GetTransform());
	player_->SetViewProjection(&followCamera_->GetCamera());

	// カメラのセットアップ
	normalCamera_ = std::make_unique<Camera>();
	normalCamera_->Initialize();
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	activeCamera_ = normalCamera_.get();

	// 音声ファイルのロード & 再生開始
	seHandle_ = audio_->LoadAudioSource("Resources/Alarm01.wav");
	bgmHandle_ = audio_->LoadAudioSource("Resources/420_long_BPM108.mp3");
	audio_->PlayWave(bgmHandle_, true, 0.5f);
	dxCommon_->SetBlendMode(blendMode_);

	textureHandle_ = TextureManager::GetInstance()->Load("resources/monsterBall.png", dxCommon_->GetCommandList());
}

void GameScene::Update() {
	// 入力更新
	input_->Update();

	// カメラ更新
	followCamera_->Update();
	normalCamera_->Update();
	debugCamera_->Update();

	//if (useDebugCamera_) {
	//	activeCamera_ = debugCamera_.get();
	//}
	//else {
	//	activeCamera_ = &followCamera_->GetCamera();
	//}
	//if (activeCamera_ != normalCamera_.get()) {
	//	activeCamera_->Update();
	//}

	// =========================================================
	// キャラクター一括更新（ポリモーフィズム）
	// =========================================================
	for (auto& character : characters_) {
		if (character) {
			character->Update(activeCamera_);
		}
	}

	// =========================================================
	// ★ 当たり判定チェック（総当り判定）[cite: 1, 8]
	// =========================================================
	for (size_t i = 0; i < characters_.size(); ++i) {
		for (size_t j = i + 1; j < characters_.size(); ++j) {
			if (!characters_[i] || !characters_[j]) continue;

			const BSphere& sphereA = characters_[i]->GetColliderSphere(); 
				const BSphere& sphereB = characters_[j]->GetColliderSphere();

				// 球 vs 球 の衝突判定[cite: 1]
				if (Physics3D::IsCollision(sphereA, sphereB)) {
					// 衝突時のイベント（デバッグログ表示や演出など）
					 OutputDebugStringA("Collision Detected!\n");
				}
		}
	}

	if (enemy_ && player_) {
		const auto& bullets = enemy_->GetBullets();
		for (const auto& bullet : bullets) {
			if (Physics3D::IsCollision(bullet->GetColliderSphere(), player_->GetColliderSphere())) {
				OutputDebugStringA("Player hit by Enemy Bullet!\n");
				// ダメージ処理などを記述
			}
		}
	}


#ifdef _DEBUG
	// デバッググリッド設定
	DebugRenderer::AddGrid(100.0f, 10, { 0.5f, 0.5f, 0.5f, 1.0f });

	Camera* gameCamera = &followCamera_->GetCamera(); // 実際に使用されているゲーム用カメラ

	if (useDebugCamera_) {
		activeCamera_ = debugCamera_.get();
	}
	else {
		activeCamera_ = gameCamera;
	}
#endif

	// 背景・エフェクトの更新
	skydome_->Update(activeCamera_);
	ground_->Update(activeCamera_);
	particle_->Update(activeCamera_);

#ifdef _DEBUG
	ImGui::Begin("Setting");
	ImGui::Checkbox("debugcamera", &useDebugCamera_);
	ImGui::End();
#endif
}

void GameScene::Draw() {

	dxCommon_->SetPipelineType(PipelineType::kObject3D);
	// 描画前処理
	dxCommon_->PreDraw();

	dxCommon_->SetPipeline(PipelineType::kObject3D, blendMode_, DepthWrite::kEnable);

	// 背景の描画
	skydome_->Draw();
	ground_->Draw();

	// =========================================================
	// キャラクター一括描画（ポリモーフィズム）
	// =========================================================
	for (const auto& character : characters_) {
		if (character) {
			character->Draw();
		}
	}

	dxCommon_->SetPipeline(PipelineType::kParticle, BlendMode::kAdd, DepthWrite::kDisable);

	particle_->Draw();

	if (useDebugCamera_) {
		debugCamera_->DrawFrustum(normalCamera_.get());
	}

#ifdef _DEBUG
	// デバッグレンダラーの描画適用
	DebugRenderer::Flush(activeCamera_);
#endif

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