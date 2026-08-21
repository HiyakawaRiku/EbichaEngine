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
	input_->Update();
	followCamera_->Update();
	normalCamera_->Update();
	debugCamera_->Update();

	// 1. キャラクターの更新
	for (auto& character : characters_) {
		if (character) character->Update(activeCamera_);
	}

	// 2. HatSphere の生成リクエスト受け取り (1回だけに整理)
	constexpr size_t kMaxHatSpheres = 15; // 必要に応じて最大数を調整
	if (enemy_) {
		Vector3 spawnPos, spawnVel;
		while (enemy_->PopHatSphereSpawnRequest(spawnPos, spawnVel)) {
			if (hatSpheres_.size() >= kMaxHatSpheres) break;

			auto hatSphere = std::make_unique<HatSphere>();
			hatSphere->Initialize(spawnPos);
			hatSphere->Throw(spawnVel, false);

			hatSpheres_.push_back(hatSphere.get());
			characters_.push_back(std::move(hatSphere));
		}
	}

	// 3. 当たり判定処理
	if (isCollisionEnabled_) {
		for (HatSphere* hat : hatSpheres_) {
			if (!hat || hat->GetState() != HatSphere::State::Thrown) continue;

			BSphere hatSphereCollider = hat->GetBSphere();

			// A. vs Player
			if (player_ && !player_->IsDead()) {
				AABB playerAABB = player_->GetColliderAABB();
				if (Physics3D::IsCollision(playerAABB, hatSphereCollider)) {
					player_->TakeDamage(1);
					hat->OnHit();
				}
			}

			// B. vs Enemy（★ 敵自身の攻撃で自爆しないように弾くか判定を除外）
			/*
			   敵が出した HatSphere で敵自身にダメージを与えないようにする場合は
			   以下の enemy_ との判定ブロックをコメントアウトまたは削除します。
			*/
			/*
			if (enemy_ && !enemy_->IsDead()) {
				AABB enemyAABB = enemy_->GetAABB();
				if (Physics3D::IsCollision(enemyAABB, hatSphereCollider)) {
					// enemy_->TakeDamage(2); // ★ 自爆の原因になるため敵へのダメージをオフ
					// hat->OnHit();
				}
			}
			*/
		}

		// C. Player vs 地面の HatSphere（拾う判定）
		if (player_ && !player_->IsDead()) {
			bool isAlreadyEquipped = std::any_of(hatSpheres_.begin(), hatSpheres_.end(),
				[](const HatSphere* h) { return h && h->GetState() == HatSphere::State::Equipped; });

			if (!isAlreadyEquipped) {
				for (auto* hatSphere : hatSpheres_) {
					if (hatSphere && hatSphere->GetState() == HatSphere::State::OnGround) {
						if (Physics3D::IsCollision(player_->GetColliderSphere(), hatSphere->GetColliderSphere())) {
							hatSphere->EquipToPlayer(player_);
							break;
						}
					}
				}
			}
		}

		// D. Player vs Enemy
		if (player_ && enemy_ && !player_->IsDead() && !enemy_->IsDead()) {
			if (Physics3D::IsCollision(player_->GetColliderSphere(), enemy_->GetColliderSphere())) {
				if (player_->IsDashAttacking()) {
					enemy_->TakeDamage(3);
				}
				else if (player_->IsJumping() && player_->GetJumpVelocityY() < 0.0f) {
					enemy_->TakeDamage(5);
				}
				else if (enemy_->IsAttacking()) {
					player_->TakeDamage(2);
				}
				else if (!player_->IsInvincible()) {
					player_->TakeDamage(1);
				}
			}
		}
	}

	// 4. Eキーで投げる処理
	if (input_->TriggerKey(DIK_E) && player_) {
		for (auto* hatSphere : hatSpheres_) {
			if (hatSphere && hatSphere->GetState() == HatSphere::State::Equipped) {
				Vector3 throwDir = { 0.0f, 0.0f, 0.0f };
				if (player_->IsMoving()) {
					Vector3 moveVel = player_->GetMoveVelocity();
					float length = std::sqrt(moveVel.x * moveVel.x + moveVel.z * moveVel.z);
					if (length > 0.0001f) throwDir = { moveVel.x / length, 0.0f, moveVel.z / length };
				}
				else {
					float rotY = player_->GetTransform().rotate.y;
					throwDir = { std::sin(rotY), 0.0f, std::cos(rotY) };
				}

				hatSphere->Throw({ throwDir.x * 0.8f, 0.2f, throwDir.z * 0.8f }, true);
				break;
			}
		}
	}

	// 5. 死亡オブジェクトの破棄処理
	std::vector<HatSphere*> deadSpheres;
	for (auto* h : hatSpheres_) {
		if (h && h->IsDead()) deadSpheres.push_back(h);
	}

	characters_.erase(
		std::remove_if(characters_.begin(), characters_.end(),
			[&deadSpheres](const std::unique_ptr<BaseCharacter>& c) {
				return std::find(deadSpheres.begin(), deadSpheres.end(), c.get()) != deadSpheres.end();
			}),
		characters_.end()
	);

	hatSpheres_.erase(
		std::remove_if(hatSpheres_.begin(), hatSpheres_.end(),
			[](const HatSphere* h) { return h == nullptr || h->IsDead(); }),
		hatSpheres_.end()
	);

	// =========================================================
	// ★ 当たり判定およびダメージ処理（Player vs Enemy）
	// =========================================================
	if (isCollisionEnabled_ && player_ && enemy_ && !player_->IsDead() && !enemy_->IsDead()) { // ★ フラグ判定を追加
		const BSphere& playerSphere = player_->GetColliderSphere();
		const BSphere& enemySphere = enemy_->GetColliderSphere();

		if (Physics3D::IsCollision(playerSphere, enemySphere)) {
			if (player_->IsDashAttacking()) {
				enemy_->TakeDamage(3);
			}
			else if (player_->IsJumping() && player_->GetJumpVelocityY() < 0.0f) {
				enemy_->TakeDamage(5);
			}
			else if (enemy_->IsAttacking()) {
				player_->TakeDamage(2);
			}
			else if (!player_->IsInvincible()) {
				player_->TakeDamage(1);
			}
		}
	}

#ifdef _DEBUG
	DebugRenderer::AddGrid(100.0f, 10, { 0.5f, 0.5f, 0.5f, 1.0f });

	if (showColliders_) {
		// 1. Player の判定可視化
		if (player_ && !player_->IsDead()) {

			// BSphere (黄)
			BSphere playerSphere = player_->GetColliderSphere();
			DebugRenderer::AddWireSphere(playerSphere.center, playerSphere.radius, 12, { 1.0f, 1.0f, 0.0f, 1.0f });

			// Dash Attack OBB (赤 - 突進時のみ)
			if (player_->IsDashAttacking()) {
				OBB dashOBB = player_->GetDashAttackOBB();
				DebugRenderer::AddWireOBB(dashOBB.transform, dashOBB.extents, { 1.0f, 0.0f, 0.0f, 1.0f });
			}
		}

		// 2. Enemy の判定可視化 (赤)
		if (enemy_ && !enemy_->IsDead()) {
			AABB enemyAABB = enemy_->GetAABB();
			DebugRenderer::AddWireAABB(enemyAABB.min, enemyAABB.max, { 1.0f, 0.2f, 0.2f, 1.0f });
		}

		// 3. HatSphere の判定可視化 (水色)
		for (HatSphere* hat : hatSpheres_) {
			if (hat) {
				BSphere hs = hat->GetColliderSphere();
				DebugRenderer::AddWireSphere(hs.center, hs.radius, 8, { 0.0f, 0.8f, 1.0f, 1.0f });
			}
		}
	}

	// =========================================================
	// ★ ImGui 設定ウィンドウ（調整機能）
	// =========================================================
	ImGui::Begin("Setting");
	ImGui::Checkbox("Debug Camera", &useDebugCamera_);
	ImGui::Checkbox("Enable Collision", &isCollisionEnabled_);
	ImGui::Checkbox("Show Colliders Wireframe", &showColliders_);

	if (player_) {
		float pRadius = player_->GetColliderSphere().radius;
		if (ImGui::DragFloat("Player Sphere Radius", &pRadius, 0.05f, 0.1f, 10.0f, "%.2f")) {
			player_->SetColliderRadius(pRadius);
		}
	}

	if (enemy_) {
		Vector3& enemyHalfSize = enemy_->GetHalfSize();
		ImGui::DragFloat3("Enemy AABB Extents", &enemyHalfSize.x, 0.05f, 0.1f, 10.0f, "%.2f");
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