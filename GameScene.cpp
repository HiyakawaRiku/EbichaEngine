#include "GameScene.h"

void GameScene::Initialize() {

	// =========================================================
	// キャラクターの生成と一括管理コンテナへの追加
	// =========================================================
	characters_.clear();
	hatSpheres_.clear();

	// 1. プレイヤーの生成・初期化
	auto player = std::make_unique<Player>();
	player->Initialize();
	player_ = player.get();
	characters_.push_back(std::move(player));

	// 2. 敵生成
	auto enemy = std::make_unique<Enemy>();
	enemy->Initialize();
	enemy->SetTargetPlayer(player_);
	enemy_ = enemy.get();
	characters_.push_back(std::move(enemy));

	// 3. HatSphere をランダムな位置に複数生成 (5個)
	constexpr int kSpawnCount = 5;
	for (int i = 0; i < kSpawnCount; ++i) {
		auto hatSphere = std::make_unique<HatSphere>();
		Vector3 randomPos = {
			RandomFloat(-10.0f, 10.0f),
			1.0f,
			RandomFloat(-10.0f, 10.0f)
		};
		hatSphere->Initialize(randomPos);

		hatSpheres_.push_back(hatSphere.get());
		characters_.push_back(std::move(hatSphere));
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
	seHandle_ = Audio::GetInstance()->LoadAudioSource("Resources/Alarm01.wav");
	bgmHandle_ = Audio::GetInstance()->LoadAudioSource("Resources/420_long_BPM108.mp3");
	Audio::GetInstance()->PlayWave(bgmHandle_, true, 0.5f);
	DirectXCommon::GetInstance()->SetBlendMode(blendMode_);

	textureHandle_ = TextureManager::GetInstance()->Load("resources/monsterBall.png", DirectXCommon::GetInstance()->GetCommandList());

	finished_ = false;
	dead_ = false;

	followCamera_->Update();
	skydome_->Update(activeCamera_);
	ground_->Update(activeCamera_);
}

void GameScene::Update() {
	followCamera_->Update();
	normalCamera_->Update();
	debugCamera_->Update();

	// 1. キャラクターの更新
	for (auto& character : characters_) {
		if (character) character->Update(activeCamera_);
	}

	// 2. 敵からの生成リクエスト処理
	UpdateEnemySpawnRequests();

	// 3. 当たり判定処理
	if (isCollisionEnabled_) {
		UpdateCollisions();
		CheckDangerousSphereCollisions();
	}

	// 4. 投げ入力処理
	UpdatePlayerThrowInput();

	// 5. 死亡オブジェクトの破棄処理
	RemoveDeadObjects();

	// 6. シーン状態更新
	if (enemy_ && enemy_->IsDead()) finished_ = true;
	if (player_ && player_->IsDead()) dead_ = true;

#ifdef _DEBUG
	DrawDebugGui();
#endif

	// カメラ切替
	activeCamera_ = useDebugCamera_ ? debugCamera_.get() : &followCamera_->GetCamera();

	// 背景・エフェクトの更新
	skydome_->Update(activeCamera_);
	ground_->Update(activeCamera_);
	particle_->Update(activeCamera_);
}

void GameScene::Draw() {
	DirectXCommon::GetInstance()->SetBlendMode(blendMode_);
	DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, blendMode_, DepthWrite::kEnable);

	// 背景の描画
	if (skydome_) skydome_->Draw();
	if (ground_) ground_->Draw();

	// キャラクター一括描画
	for (const auto& character : characters_) {
		if (character) character->Draw();
	}

	// エフェクト描画
	DirectXCommon::GetInstance()->SetPipeline(PipelineType::kParticle, BlendMode::kAdd, DepthWrite::kDisable);
	particle_->Draw();

	if (useDebugCamera_) {
		debugCamera_->DrawFrustum(normalCamera_.get());
	}

#ifdef _DEBUG
	DebugRenderer::Flush(activeCamera_);
#endif
}

void GameScene::Finalize() {
	if (Audio::GetInstance()) {
		Audio::GetInstance()->Unload(bgmHandle_);
		Audio::GetInstance()->Unload(seHandle_);
	}
}

// ---------------------------------------------------------
// 内部サブ関数群
// ---------------------------------------------------------

void GameScene::UpdateEnemySpawnRequests() {
	constexpr size_t kMaxHatSpheres = 15;
	if (!enemy_) return;

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

void GameScene::UpdateCollisions() {
	// A & B. HatSphere 関連判定
	for (HatSphere* hat : hatSpheres_) {
		if (!hat || hat->GetState() != HatSphere::State::Thrown) continue;

		BSphere hatSphereCollider = hat->GetBSphere();

		// 敵が吐いた HatSphere -> プレイヤー
		if (!hat->IsThrownByPlayer()) {
			if (player_ && !player_->IsDead()) {
				if (Physics3D::IsCollision(player_->GetColliderAABB(), hatSphereCollider)) {
					player_->TakeDamage(1);
					hat->OnHit();
				}
			}
		}
		// プレイヤーが投げた HatSphere -> 敵
		else {
			if (enemy_ && !enemy_->IsDead()) {
				if (Physics3D::IsCollision(enemy_->GetAABB(), hatSphereCollider)) {
					Vector3 spherePos = hat->GetTransform().translate;
					Vector3 enemyPos = enemy_->GetTransform().translate;
					Vector3 knockDir = enemyPos - spherePos;
					knockDir.y = 0.0f;

					float len = std::sqrt(knockDir.x * knockDir.x + knockDir.z * knockDir.z);
					knockDir = (len > 0.0001f) ? Vector3{ knockDir.x / len, 0.0f, knockDir.z / len } : Vector3{ 0.0f, 0.0f, 1.0f };

					enemy_->TakeDamage(3, { knockDir.x * 0.5f, 0.1f, knockDir.z * 0.5f });
					hat->OnHit();
				}
			}
		}
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

	// D. Player vs Enemy（直接衝突判定）
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

void GameScene::CheckDangerousSphereCollisions() {
	if (!player_) return;

	Vector3 playerPos = player_->GetTransform().translate;
	constexpr float kPlayerRadius = 0.8f;

	for (auto* hatSphere : hatSpheres_) {
		if (!hatSphere || !hatSphere->IsDangerous()) continue;

		Vector3 spherePos = hatSphere->GetTransform().translate;
		Vector3 diff = playerPos - spherePos;
		diff.y = 0.0f;

		float distSq = diff.x * diff.x + diff.z * diff.z;
		constexpr float kMinDist = kPlayerRadius + 0.5f;

		if (distSq <= kMinDist * kMinDist) {
			float dist = std::sqrt(distSq);
			Vector3 knockDir = (dist > 0.0001f)
				? Vector3{ diff.x / dist, 0.0f, diff.z / dist }
			: Vector3{ 0.0f, 0.0f, 1.0f };

			player_->ApplyKnockback({ knockDir.x * 0.8f, 0.2f, knockDir.z * 0.8f });
			hatSphere->OnHitPlayer({ -knockDir.x * 0.4f, 0.25f, -knockDir.z * 0.4f });
		}
	}
}

void GameScene::UpdatePlayerThrowInput() {
	if (Input::GetInstance()->TriggerKey(DIK_E) && player_) {
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
}

void GameScene::RemoveDeadObjects() {
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
}

#ifdef _DEBUG
void GameScene::DrawDebugGui() {
	DebugRenderer::AddGrid(100.0f, 10, { 0.5f, 0.5f, 0.5f, 1.0f });

	if (showColliders_) {
		if (player_ && !player_->IsDead()) {
			BSphere playerSphere = player_->GetColliderSphere();
			DebugRenderer::AddWireSphere(playerSphere.center, playerSphere.radius, 12, { 1.0f, 1.0f, 0.0f, 1.0f });

			if (player_->IsDashAttacking()) {
				OBB dashOBB = player_->GetDashAttackOBB();
				DebugRenderer::AddWireOBB(dashOBB.transform, dashOBB.extents, { 1.0f, 0.0f, 0.0f, 1.0f });
			}
		}

		if (enemy_ && !enemy_->IsDead()) {
			AABB enemyAABB = enemy_->GetAABB();
			DebugRenderer::AddWireAABB(enemyAABB.min, enemyAABB.max, { 1.0f, 0.2f, 0.2f, 1.0f });
		}

		for (HatSphere* hat : hatSpheres_) {
			if (hat) {
				BSphere hs = hat->GetColliderSphere();
				DebugRenderer::AddWireSphere(hs.center, hs.radius, 8, { 0.0f, 0.8f, 1.0f, 1.0f });
			}
		}
	}

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

	if (enemy_) {
		ImGui::Begin("Enemy Status");
		int currentHp = enemy_->GetHp();
		int maxHp = enemy_->GetMaxHp();
		float hpRatio = (float)currentHp / (float)maxHp;

		if (enemy_->IsDead()) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STATUS: DEAD");
		}
		else {
			ImGui::Text("STATUS: ALIVE");
		}

		ImGui::Text("HP: %d / %d", currentHp, maxHp);
		ImGui::ProgressBar(hpRatio, ImVec2(-1.0f, 0.0f));

		ImGui::Separator();
		if (ImGui::Button("Deal 5 Damage")) {
			enemy_->TakeDamage(5, { 0.0f, 0.0f, -0.3f });
		}

		Vector3 enemyPos = enemy_->GetTransform().translate;
		ImGui::Text("Pos: X:%.2f, Y:%.2f, Z:%.2f", enemyPos.x, enemyPos.y, enemyPos.z);
		ImGui::End();
	}
}
#endif