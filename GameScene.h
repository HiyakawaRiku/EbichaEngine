#pragma once
#include <memory>
#include <cstdint>
#include <vector>
#include <random>

// エンジンヘッダー
#include "EbichaEngine.h"

// キャラクター基底および派生クラス
#include "BaseCharacter.h"
#include "Player.h"
#include "Enemy.h"

// その他のオブジェクト
#include "Skydome.h"
#include "Ground.h"
#include "FollowCamera.h"
#include "Particle.h"
#include "HatSphere.h"

class GameScene {
public:
	GameScene() = default;
	~GameScene() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了処理・解放
	/// </summary>
	void Finalize();

	// getter
	bool IsFinished() const { return finished_; }
	bool IsDead() const { return dead_; }

private:
	// 内部処理切り出し関数
	void UpdateEnemySpawnRequests();
	void UpdateCollisions();
	void UpdatePlayerThrowInput();
	void RemoveDeadObjects();
	void CheckDangerousSphereCollisions();

#ifdef _DEBUG
	void DrawDebugGui();
#endif

	// 補助関数: 指定範囲のランダムな float を取得
	float RandomFloat(float min, float max) {
		std::uniform_real_distribution<float> dist(min, max);
		return dist(randomEngine_);
	}

private:
	bool dead_ = false;
	bool finished_ = false; // 終了フラグ


	// ランダム生成用
	std::random_device seedGenerator_;
	std::mt19937 randomEngine_{ seedGenerator_() };

	// カメラ関連
	std::unique_ptr<Camera> normalCamera_;
	std::unique_ptr<DebugCamera> debugCamera_;
	Camera* activeCamera_ = nullptr;
	bool useDebugCamera_ = false;

	// デバッグ・判定フラグ
	bool isCollisionEnabled_ = true;
	bool showColliders_ = true;

	// オーディオハンドル & テクスチャ
	uint32_t seHandle_ = 0;
	uint32_t bgmHandle_ = 0;
	TextureHandle textureHandle_;

	BlendMode blendMode_ = BlendMode::kNormal;

	// =========================================================
	// キャラクター一括管理
	// =========================================================
	std::vector<std::unique_ptr<BaseCharacter>> characters_;

	// 参照ポインタ（所有権は characters_ が保持）
	Player* player_ = nullptr;
	Enemy* enemy_ = nullptr;

	std::vector<HatSphere*> hatSpheres_;

	// 背景・カメラ・エフェクトなど
	std::unique_ptr<Skydome> skydome_ = nullptr;
	std::unique_ptr<Ground> ground_ = nullptr;
	std::unique_ptr<FollowCamera> followCamera_ = nullptr;
	std::unique_ptr<Particle> particle_ = nullptr;
};