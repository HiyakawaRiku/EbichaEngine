#pragma once
#include <memory>
#include <cstdint>
#include <vector>

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

private:

	// エンジンの各ポインタ（シングルトン参照）
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	// ゲームオブジェクト（スプライト等）
	std::unique_ptr<Sprite> sprite_;

	// カメラ関連
	std::unique_ptr<Camera> normalCamera_;
	std::unique_ptr<DebugCamera> debugCamera_;
	Camera* activeCamera_ = nullptr;
	bool useDebugCamera_ = false;

	// Transformデータ
	Transform transformSprite_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform uvTransformSprite_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// オーディオハンドル
	uint32_t seHandle_ = 0;
	uint32_t bgmHandle_ = 0;
	TextureHandle textureHandle_;

	BlendMode blendMode_ = BlendMode::kNormal;
	const char* blendModeNames_ = "none\0normal\0add\0subtract\0multiply\0screen\0\0";

private:

	// =========================================================
	// キャラクター一括管理（ポリモーフィズム）
	// =========================================================
	std::vector<std::unique_ptr<BaseCharacter>> characters_;

	// FollowCamera へのターゲット参照用（所有権は characters_ が保持）
	Player* player_ = nullptr;
	Enemy* enemy_ = nullptr;

	// 背景・カメラ・エフェクトなど
	std::unique_ptr<Skydome> skydome_ = nullptr;
	std::unique_ptr<Ground> ground_ = nullptr;
	std::unique_ptr<FollowCamera> followCamera_ = nullptr;
	std::unique_ptr<Particle> particle_ = nullptr;
};