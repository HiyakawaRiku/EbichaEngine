#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <chrono>

#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ModelCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "Sound.h"
#include "ParticleManager.h"
#include "D3DResourceLeakChecker.h"

#include "SceneManager.h"

class EbichaEngine {
public:
    EbichaEngine() = default;
    ~EbichaEngine() = default;

    void Initialize();
    void Run();
    void Update();
    void Draw();
    void Finalize();

private:
    void InitializeLog();

private:
    D3DResourceLeakChecker leakCheck_;

    // 基盤・マネージャー類
    std::unique_ptr<WinApp> winApp_;
    std::unique_ptr<DirectXCommon> dxCommon_;
    std::unique_ptr<Input> input_;
    std::unique_ptr<SpriteCommon> spriteCommon_;
    std::unique_ptr<Object3dCommon> object3dCommon_;
    std::unique_ptr<ModelCommon> modelCommon_;
    std::unique_ptr<SrvManager> srvManager_;
    std::unique_ptr<ImGuiManager> imGuiManager_;
    std::unique_ptr<Sound> sound_;

    

    std::ofstream logStream_;
};