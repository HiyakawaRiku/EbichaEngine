#include "GameScene.h"

void GameScene::Initialize() {
	phase_ = Phase::kExplain;

	// ExplainLogo 初期化
	explainLogo_ = std::make_unique<ExplainLogo>();
	explainLogo_->Initialize();

	// 1. FadeManagerと同様に白テクスチャをロード[cite: 1]
	whiteTexture_ = TextureManager::GetInstance()->Load("resources/white1280x720.png", DirectXCommon::GetInstance()->GetCommandList());

	// ポーズ背景用半透明オーバーレイ
	pauseSprite_ = std::make_unique<Sprite>();
	pauseSprite_->Initialize();
	pauseSprite_->size = { 1280.0f, 720.0f };
	pauseSprite_->transform.translate = { 0.0f, 0.0f, 0.0f };
	pauseSprite_->color = { 0.0f, 0.0f, 0.0f, 0.5f }; // 黒の半透明

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

	// バーの表示位置と基本サイズの設定
	const Vector3 kBarPos = { 440.0f, 40.0f ,0.0f }; // 画面中央上部など
	const Vector2 kBarSize = { 400.0f, 20.0f }; // 幅400px, 高さ20px

	// 2. HPバー背景（黒色の枠）
	enemyHpBarBg_ = std::make_unique<Sprite>();
	enemyHpBarBg_->Initialize();
	enemyHpBarBg_->transform.translate = kBarPos;
	enemyHpBarBg_->size = kBarSize;
	enemyHpBarBg_->color = { 0.1f, 0.1f, 0.1f, 0.8f }; // 黒色（半透明）

	// 3. HPバー本体（赤色）
	enemyHpBarFill_ = std::make_unique<Sprite>();
	enemyHpBarFill_->Initialize();
	enemyHpBarFill_->transform.translate = kBarPos;
	enemyHpBarFill_->size = kBarSize;
	enemyHpBarFill_->color = { 0.9f, 0.1f, 0.1f, 1.0f }; // 赤色

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

	textureHeart_ = TextureManager::GetInstance()->Load("resources/heart.png", DirectXCommon::GetInstance()->GetCommandList());

	// ハートSprite（3つ）の初期化
	heartSprites_.clear();
	const Vector2 kInitialPos = { 30.0f, 600.0f }; // 1つ目のハートの位置 (X, Y)
	const float kSpacing = 64.0f;                 // ハート同士の間隔

	for (int i = 0; i < kHeartCount; ++i) {
		auto sprite = std::make_unique<Sprite>();
		sprite->Initialize();
		sprite->size = { 100.0f, 100.0f }; // ハートのサイズ
		sprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		sprite->transform.translate = { kInitialPos.x + (i * kSpacing), kInitialPos.y };

		heartSprites_.push_back(std::move(sprite));
	}

	finished_ = false;
	dead_ = false;

	followCamera_->Update();
	skydome_->Update(activeCamera_);
	ground_->Update(activeCamera_);

	// 1. キャラクターの更新
	for (auto& character : characters_) {
		if (character) character->Update(activeCamera_);
	}

	// 1. テクスチャのロード (FadeManagerと同様にTextureManagerを使用)
	uiTexture_ = TextureManager::GetInstance()->Load("resources/heart.png", DirectXCommon::GetInstance()->GetCommandList());

	//// 2. Spriteの生成と初期化
	//uiSprite_ = std::make_unique<Sprite>();
	//uiSprite_->Initialize();
	//uiSprite_->size = { 64.0f, 64.0f };   // 任意のサイズを指定
	//uiSprite_->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // カラー・アルファ値

	uiTexture_ = TextureManager::GetInstance()->Load("resources/explaintext.png", DirectXCommon::GetInstance()->GetCommandList());

	// 2. Spriteの生成と初期化
	uiSprite_ = std::make_unique<Sprite>();
	uiSprite_->Initialize();
	uiSprite_->size = { 1280.0f, 720.0f };                 // 画像サイズに合わせて適宜調整してください
	//uiSprite_->transform.translate = { 320.0f, 180.0f, 0.0f }; // 画面中央付近に配置（座標は適宜調整）
	uiSprite_->color = { 1.0f, 1.0f, 1.0f, 1.0f };

// 仮のポーズテキストスプライト初期化 (resources/explaintext.png を仮使用)
	pauseTextTexture_ = TextureManager::GetInstance()->Load("resources/pausetext.png", DirectXCommon::GetInstance()->GetCommandList());

	pauseTextSprite_ = std::make_unique<Sprite>();
	pauseTextSprite_->Initialize();
	pauseTextSprite_->size = { 640.0f, 360.0f }; // 適宜調整
	pauseTextSprite_->transform.translate = { 320.0f, 180.0f, 0.0f }; // 画面中央付近
	pauseTextSprite_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void GameScene::Update() {
	// カメラ切り替え
	activeCamera_ = useDebugCamera_ ? debugCamera_.get() : &followCamera_->GetCamera();

	// ---------------------------------------------------------
	// 1. 説明フェーズおよびポーズフェーズの処理
	// ---------------------------------------------------------
	if (phase_ == Phase::kExplain) {
		if (explainLogo_) {
			explainLogo_->Update(activeCamera_);
		}

		// ENTERキーまたはSPACEキーでゲーム開始
		if (Input::GetInstance()->TriggerKey(DIK_RETURN) || Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			phase_ = Phase::kPlaying;
			explainLogo_.reset(); // 説明用ロゴのメモリ解放
		}

		// 背景のみ動かし、ゲームオブジェクトの更新は停止
		skydome_->Update(activeCamera_);
		ground_->Update(activeCamera_);
		return;
	}
	else if (phase_ == Phase::kPlaying) {
		// Pキー または ESCキーでポーズ画面へ
		if (Input::GetInstance()->TriggerKey(DIK_P) || Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
			phase_ = Phase::kPause;
		}
	}
	else if (phase_ == Phase::kPause) {
		// Pキー または ESCキーでポーズ解除
		if (Input::GetInstance()->TriggerKey(DIK_P) || Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
			phase_ = Phase::kPlaying;
		}
		// Enterキーでタイトルへ戻る
		else if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			isReturnToTitle_ = true;
			return;
		}

		if (pauseSprite_) {
			pauseSprite_->Update();
		}
		if (pauseTextSprite_) {
			pauseTextSprite_->Update();
		}

		// ポーズ中はゲームの更新をストップ
		return;
	}

	// ---------------------------------------------------------
	// 2. 通常プレイ時 (Phase::kPlaying) の更新
	// ---------------------------------------------------------
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

	// 背景・エフェクトの更新
	skydome_->Update(activeCamera_);
	ground_->Update(activeCamera_);
	particle_->Update(activeCamera_);

	if (uiSprite_) {
		uiSprite_->Update();
	}

	if (player_) {
		int currentHp = player_->GetHp();
		int maxHp = player_->GetMaxHp();

		// 1つあたりのハートが表すHP閾値
		float hpPerHeart = static_cast<float>(maxHp) / kHeartCount;

		for (int i = 0; i < kHeartCount; ++i) {
			if (!heartSprites_[i]) continue;

			float thresholdHp = i * hpPerHeart;

			if (currentHp > thresholdHp) {
				heartSprites_[i]->color.w = 1.0f;
			}
			else {
				heartSprites_[i]->color.w = 0.2f;
			}

			heartSprites_[i]->Update();
		}
	}

	// 敵のHPバー更新
	if (enemy_ && enemyHpBarFill_) {
		int currentHp = enemy_->GetHp();
		int maxHp = enemy_->GetMaxHp();

		// HP割合の計算（0.0f ～ 1.0f）
		float hpRate = static_cast<float>(currentHp) / static_cast<float>(maxHp);
		if (hpRate < 0.0f) hpRate = 0.0f;

		// 最大幅（400px）に対してHP割合を掛けて幅を変更
		constexpr float kMaxBarWidth = 400.0f;
		enemyHpBarFill_->size.x = kMaxBarWidth * hpRate;

		// Spriteの頂点データ・定数バッファの更新[cite: 1]
		enemyHpBarBg_->Update();
		enemyHpBarFill_->Update();
	}
}

void GameScene::Draw() {
	DirectXCommon::GetInstance()->SetBlendMode(blendMode_);
	DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, blendMode_, DepthWrite::kEnable);

	if (phase_ == Phase::kPause) {
		DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kNormal, DepthWrite::kDisable);

		// 背景オーバーレイ
		if (pauseSprite_) {
			pauseSprite_->Draw(whiteTexture_);
		}
		// ポーズテキスト（仮スプライト）
		if (pauseTextSprite_) {
			pauseTextSprite_->Draw(pauseTextTexture_);
		}
	}

	// 背景の描画
	if (skydome_) skydome_->Draw();
	if (ground_) ground_->Draw();

	// 説明ロゴの描画 (kExplain フェーズのみ)
	if (phase_ == Phase::kExplain && explainLogo_) {
		explainLogo_->Draw();
	}

	// キャラクター一括描画
	for (const auto& character : characters_) {
		if (character) character->Draw();
	}

	if (uiSprite_) {
		// FadeManagerと同じパイプライン設定を使用
		DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kNormal, DepthWrite::kDisable);
		uiSprite_->Draw(uiTexture_);
	}

	for (const auto& sprite : heartSprites_) {
		if (sprite) {
			sprite->Draw(textureHeart_);
		}
	}

	if (enemy_ && !enemy_->IsDead()) {
		// FadeManagerと同じパイプライン設定を適用[cite: 1]
		DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kNormal, DepthWrite::kDisable);

		// 背景（黒） -> バー本体（赤）の順で描画
		if (enemyHpBarBg_) enemyHpBarBg_->Draw(whiteTexture_);
		if (enemyHpBarFill_) enemyHpBarFill_->Draw(whiteTexture_);
	}

	// エフェクト描画
	DirectXCommon::GetInstance()->SetPipeline(PipelineType::kParticle, BlendMode::kAdd, DepthWrite::kDisable);
	//particle_->Draw();

	// ポーズ用オーバーレイおよび文字の描画
	if (phase_ == Phase::kPause && pauseSprite_) {
		DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kNormal, DepthWrite::kDisable);
		pauseSprite_->Draw(pauseTextTexture_);

#ifdef _DEBUG
		ImGui::SetNextWindowPos(ImVec2(560.0f, 320.0f));
		ImGui::Begin("PauseMenu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("=== PAUSE ===");
		ImGui::Text("Press 'P' or 'ESC' to Resume");
		ImGui::End();
#endif
	}

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

					if (particle_) {
						particle_->EmitAt(enemy_->GetTransform().translate, 15);
					}
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
				//enemy_->TakeDamage(3);

				//if (particle_) {
				//	particle_->EmitAt(enemy_->GetTransform().translate, 20);
				//}
			}
			else if (player_->IsJumping() && player_->GetJumpVelocityY() < 0.0f) {
				//enemy_->TakeDamage(5);

				//if (particle_) {
				//	particle_->EmitAt(player_->GetTransform().translate, 25);
				//}
			}
			else if (enemy_->IsAttacking()) {
				player_->TakeDamage(2);

				if (particle_) {
					particle_->EmitAt(player_->GetTransform().translate, 25);
				}
			}
			else if (!player_->IsInvincible()) {
				player_->TakeDamage(1);

				if (particle_) {
					particle_->EmitAt(player_->GetTransform().translate, 25);
				}
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