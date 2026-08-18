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
	enemy_ = enemy.get();            // ★ 参照保持用
	characters_.push_back(std::move(enemy));

	// ★ 2. HatSphere をランダムな位置に複数生成 (例: 5個)
	int spawnCount = 5;
	for (int i = 0; i < spawnCount; ++i) {
		auto hatSphere = std::make_unique<HatSphere>();

		// X: -10~10, Y: 1, Z: -10~10 のランダム位置
		Vector3 randomPos = {
			RandomFloat(-10.0f, 10.0f),
			1.0f,
			RandomFloat(-10.0f, 10.0f)
		};
		hatSphere->Initialize(randomPos);

		hatSpheres_.push_back(hatSphere.get());      // 更新・判定用参照の保存
		characters_.push_back(std::move(hatSphere)); // 所有権を characters_ へ (自動描画用)
	}

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

	// 1. キャラクターの更新
    for (auto& character : characters_) {
        if (character) character->Update(activeCamera_);
    }

    // 2. Cube敵からの HatSphere 生成要求の受け取り
    if (enemy_) {
        Vector3 spawnPos, spawnVel;
        while (enemy_->PopHatSphereSpawnRequest(spawnPos, spawnVel)) {
            auto hatSphere = std::make_unique<HatSphere>();
            hatSphere->Initialize(spawnPos);
            hatSphere->Throw(spawnVel); // 重力運動を開始

            hatSpheres_.push_back(hatSphere.get());
            characters_.push_back(std::move(hatSphere));
        }
    }

    // 3. 当たり判定処理（Physics3D を使用）
    for (HatSphere* hat : hatSpheres_) {
        if (!hat || hat->GetState() != HatSphere::State::Thrown) continue;

        BSphere hatSphere = hat->GetBSphere();

        // --- A. HatSphere (BSphere) vs プレイヤー (AABB) ---
        if (player_) {
            AABB playerAABB = player_->GetColliderAABB();
            if (Physics3D::IsCollision(playerAABB, hatSphere)) {
                player_->TakeDamage(1);
                hat->OnHit(); // 跳ね返って縮小消滅へ
            }
        }

        // --- B. HatSphere (BSphere) vs Cube敵 (AABB) ---
        if (enemy_) {
            AABB enemyAABB = enemy_->GetAABB();
            if (Physics3D::IsCollision(enemyAABB, hatSphere)) {
                enemy_->TakeDamage(2);
                hat->OnHit();
            }
        }
    }

	// =========================================================
	// ★ 敵からの HatSphere 生成リクエスト処理
	// =========================================================
	if (enemy_) {
		Vector3 spawnPos, spawnVel;
		while (enemy_->PopHatSphereSpawnRequest(spawnPos, spawnVel)) {
			auto hatSphere = std::make_unique<HatSphere>();
			hatSphere->Initialize(spawnPos);

			// 出現直後から投げられた状態（State::Thrown）にして飛んでいかせる
			// ※ 敵が射出した方向・速度を適用するため直接投擲状態を設定するメソッドか、強引に Throw 呼出しを行います
			hatSphere->Throw({ spawnVel.x, 0.0f, spawnVel.z });

			// （補足: HatSphere に速度を直接設定できる SetVelocity 等を用意するとより正確になります）

			hatSpheres_.push_back(hatSphere.get());
			characters_.push_back(std::move(hatSphere));
		}
	}

	// =========================================================
	// ★ HatSphere の当たり判定・乗っかる処理
	// =========================================================
	if (player_) {
		// すでに頭に乗っている球があるかチェック
		bool isAlreadyEquipped = std::any_of(hatSpheres_.begin(), hatSpheres_.end(),
			[](const HatSphere* h) {
				return h && h->GetState() == HatSphere::State::Equipped;
			});

		// 乗っていない場合のみ、地面にある球と接触判定
		if (!isAlreadyEquipped) {
			for (auto* hatSphere : hatSpheres_) {
				if (hatSphere && hatSphere->GetState() == HatSphere::State::OnGround) {
					const BSphere& playerSphere = player_->GetColliderSphere();
					const BSphere& hatSphereCollider = hatSphere->GetColliderSphere();

					if (Physics3D::IsCollision(playerSphere, hatSphereCollider)) {
						hatSphere->EquipToPlayer(player_);
						break; // 1個乗ったら終了
					}
				}
			}
		}

		// =========================================================
		// ★ Eキーで投げる処理
		// =========================================================
		if (input_->TriggerKey(DIK_E)) {
			for (auto* hatSphere : hatSpheres_) {
				if (hatSphere && hatSphere->GetState() == HatSphere::State::Equipped) {
					Vector3 throwDir = { 0.0f, 0.0f, 0.0f };

					// プレイヤーが移動中の場合は移動方向へ投げる
					if (player_->IsMoving()) {
						Vector3 moveVel = player_->GetMoveVelocity();
						float length = std::sqrt(moveVel.x * moveVel.x + moveVel.z * moveVel.z);
						if (length > 0.0001f) {
							throwDir = { moveVel.x / length, 0.0f, moveVel.z / length };
						}
					}
					else {
						// 立ち止まっている場合はプレイヤーの正面向きへ投げる
						float rotY = player_->GetTransform().rotate.y;
						throwDir = { std::sin(rotY), 0.0f, std::cos(rotY) };
					}

					hatSphere->Throw(throwDir);
					break; // 1個投げたら終了
				}
			}
		}
	}

	// =========================================================
	// ★ 消滅処理 (IsDead が true になった HatSphere を削除)
	// =========================================================
	// 1. hatSpheres_ から削除された要素を取得
	std::vector<HatSphere*> deadSpheres;
	for (auto* h : hatSpheres_) {
		if (h && h->IsDead()) {
			deadSpheres.push_back(h);
		}
	}

	// 2. characters_ (unique_ptr) 内から該当オブジェクトを消去（画面から消える）
	characters_.erase(
		std::remove_if(characters_.begin(), characters_.end(),
			[&deadSpheres](const std::unique_ptr<BaseCharacter>& c) {
				return std::find(deadSpheres.begin(), deadSpheres.end(), c.get()) != deadSpheres.end();
			}),
		characters_.end()
	);

	// 3. hatSpheres_ (生ポインタ) からも消去
	hatSpheres_.erase(
		std::remove_if(hatSpheres_.begin(), hatSpheres_.end(),
			[](const HatSphere* h) {
				return h == nullptr || h->IsDead();
			}),
		hatSpheres_.end()
	);

	// =========================================================
	// ★ 当たり判定およびダメージ処理（Player vs Enemy）
	// =========================================================
	if (player_ && enemy_ && !player_->IsDead() && !enemy_->IsDead()) {

		const BSphere& playerSphere = player_->GetColliderSphere();
		const BSphere& enemySphere = enemy_->GetColliderSphere();

		// 1. 基本的な球同士の衝突チェック（接触しているか）
		if (Physics3D::IsCollision(playerSphere, enemySphere)) {

			// A. プレイヤーがダッシュ突進攻撃中 -> 敵にダメージ
			if (player_->IsDashAttacking()) {
				enemy_->TakeDamage(3);
			}
			// B. プレイヤーがジャンプ中かつ下降中（急降下攻撃）
			else if (player_->IsJumping() && player_->GetJumpVelocityY() < 0.0f) {
				// 敵がグロッキー（大技後のひるみ）状態なら特大ダメージが適用される
				enemy_->TakeDamage(5);
			}
			// C. 敵が攻撃中（突進・大技など） -> プレイヤーにダメージ
			else if (enemy_->IsAttacking()) {
				player_->TakeDamage(2);
			}
			// D. お互い通常状態での接触（体当たり） -> プレイヤーがダメージ
			else if (!player_->IsInvincible()) {
				player_->TakeDamage(1);
			}
		}
	}

#ifdef _DEBUG
	DebugRenderer::AddGrid(100.0f, 10, { 0.5f, 0.5f, 0.5f, 1.0f });
#endif

	// =========================================================
	// ★ 当たり判定の可視化・デバッグ操作
	// =========================================================
#ifdef _DEBUG
	// ImGui 設定ウィンドウ
	ImGui::Begin("Setting");
	ImGui::Checkbox("debugcamera", &useDebugCamera_);

	if (player_) {
		ImGui::Separator();
		ImGui::Text("Player Collider (AABB)");
		Vector3& extents = player_->GetBoxExtents();
		ImGui::DragFloat3("Box Half Extents", &extents.x, 0.05f, 0.1f, 10.0f, "%.2f");

		ImGui::Separator();
		ImGui::Text("Hammer Collider (AABB)");

		// サイズ調整
		Vector3& hammerExtents = player_->GetHammerBoxExtents();
		ImGui::DragFloat3("Hammer Half Extents", &hammerExtents.x, 0.05f, 0.1f, 10.0f, "%.2f");

		// 位置（オフセット）調整
		Vector3& hammerOffset = player_->GetHammerColliderOffset();
		ImGui::DragFloat3("Hammer Offset", &hammerOffset.x, 0.05f, -5.0f, 5.0f, "%.2f");
	}
	ImGui::End();
#endif

	Camera* gameCamera = &followCamera_->GetCamera(); // 実際に使用されているゲーム用カメラ

	if (useDebugCamera_) {
		activeCamera_ = debugCamera_.get();
	}
	else {
		activeCamera_ = gameCamera;
	}

	// 背景・エフェクトの更新
	skydome_->Update(activeCamera_);
	ground_->Update(activeCamera_);
	particle_->Update(activeCamera_);
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