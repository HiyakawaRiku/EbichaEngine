#include "EbichaEngine.h"
#include "GameScene.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    // エンジンの一括初期化
    EbichaEngine* ebichaEngine = EbichaEngine::GetInstance();
    ebichaEngine->Initialize();

    // ゲームシーンの生成と初期化
    auto gameScene = std::make_unique<GameScene>();
    gameScene->Initialize();

    MSG msg{};
    // メインループ
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {

#ifdef USE_IMGUI
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
#endif

            // シーンの更新と描画
            gameScene->Update();
            gameScene->Draw();


        }
    }

    // ゲームシーンの終了処理
    gameScene->Finalize();

    // エンジンの終了処理
    DebugRenderer::Finalize();
    ebichaEngine->Finalize();

    return 0;
}