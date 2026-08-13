#pragma once
#include <memory>
#include <cstdint>

// 前方宣言または必要なヘッダーのインクルード
#include "EbichaEngine.h"
#include "Player.h"
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

    // ゲームオブジェクト（生のポインタ管理）
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<Sphere> sphere_;
    //Model* modelTeapot_ = nullptr;
    //Model* modelBunny_ = nullptr;
    //Model* modelMultiMesh_ = nullptr;

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
    Player* player_=nullptr;
    Skydome* skydome_ = nullptr;
    std::unique_ptr<Ground> ground_ = nullptr;
    FollowCamera* followCamera_ = nullptr;
    Particle* particle_ = nullptr;
};