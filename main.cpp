#include "EbichaEngine.h"

// シーンヘッダー
#include "TitleScene.h"
#include "GameScene.h"
#include "GameOverScene.h"
#include "ClearScene.h"

#include "FadeManager.h"

enum class Scene {
	kUnknown = 0,
	kTitle,
	kGame,
	kGameOver,
	kClear
};

Scene scene = Scene::kUnknown;

TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;
GameOverScene* gameOverScene = nullptr;
ClearScene* clearScene = nullptr;

void ChangeScene();
void UpdateScene();
void DrawScene();

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの一括初期化
	EbichaEngine* ebichaEngine = EbichaEngine::GetInstance();
	ebichaEngine->Initialize();

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	TextureManager::GetInstance()->Initialize(dxCommon->GetDevice(), dxCommon->GetSrvHeap());

	Audio* audio_ = Audio::GetInstance();
	if (audio_) {
		audio_->Initialize();
	}

	// 初期シーンの設定
	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();

	FadeManager::GetInstance()->Initialize();

	MSG msg{};

	// メインループ
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// 描画前処理
			DirectXCommon::GetInstance()->PreDraw();

#ifdef USE_IMGUI
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
#endif
			Input::GetInstance()->Update();
			FadeManager::GetInstance()->Update();


			// シーンの更新と描画
			ChangeScene();
			UpdateScene();
			DrawScene();
			FadeManager::GetInstance()->Draw();

			// 描画後処理
			DirectXCommon::GetInstance()->PostDraw();
		}
	}

	// 終了時に残っているシーンを解放
	if (titleScene) {
		titleScene->Finalize();
		delete titleScene;
		titleScene = nullptr;
	}
	if (gameScene) {
		gameScene->Finalize();
		delete gameScene;
		gameScene = nullptr;
	}
	if (gameOverScene) {
		gameOverScene->Finalize();
		delete gameOverScene;
		gameOverScene = nullptr;
	}
	if (clearScene) {
		clearScene->Finalize();
		delete clearScene;
		clearScene = nullptr;
	}

	// エンジンの終了処理
	DebugRenderer::Finalize();
	ebichaEngine->Finalize();

	return 0;
}

void ChangeScene()
{
	switch (scene) {
	case Scene::kTitle:
		if (titleScene && titleScene->IsFinished()) {
			titleScene->Finalize();
			delete titleScene;
			titleScene = nullptr;

			scene = Scene::kGame;
			gameScene = new GameScene();
			gameScene->Initialize();

			// 新しいシーンに切り替わったらフェードインを開始
			FadeManager::GetInstance()->StartFadeIn(0.5f);
			return;
		}
		break;

	case Scene::kGame:
		if (gameScene && gameScene->IsDead()) {
			gameScene->Finalize();
			delete gameScene;
			gameScene = nullptr;

			scene = Scene::kGameOver;
			gameOverScene = new GameOverScene();
			gameOverScene->Initialize();

			FadeManager::GetInstance()->StartFadeIn(0.5f);
			return;
		}
		else if (gameScene && gameScene->IsFinished()) {
			gameScene->Finalize();
			delete gameScene;
			gameScene = nullptr;

			scene = Scene::kClear;
			clearScene = new ClearScene();
			clearScene->Initialize();

			FadeManager::GetInstance()->StartFadeIn(0.5f);
			return;
		}
		break;
	case Scene::kGameOver:
		if (gameOverScene && gameOverScene->IsFinished()) {
			gameOverScene->Finalize();
			delete gameOverScene;
			gameOverScene = nullptr;

			scene = Scene::kTitle;
			titleScene = new TitleScene();
			titleScene->Initialize();

			// 新しいシーンに切り替わったらフェードインを開始
			FadeManager::GetInstance()->StartFadeIn(0.5f);
			return;
		}
		break;
	case Scene::kClear:
		if (clearScene && clearScene->IsFinished()) {
			clearScene->Finalize();
			delete clearScene;
			clearScene = nullptr;

			scene = Scene::kTitle;
			titleScene = new TitleScene();
			titleScene->Initialize();

			// 新しいシーンに切り替わったらフェードインを開始
			FadeManager::GetInstance()->StartFadeIn(0.5f);
			return;
		}
		break;
		// GameOver, Clear も同様に FadeManager::GetInstance()->StartFadeIn(0.5f); を挿入
	}
}

void UpdateScene()
{
	switch (scene) {
	case Scene::kTitle:
		if (titleScene) titleScene->Update();
		break;
	case Scene::kGame:
		if (gameScene) gameScene->Update();
		break;
	case Scene::kGameOver:
		if (gameOverScene) gameOverScene->Update();
		break;
	case Scene::kClear:
		if (clearScene) clearScene->Update();
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene) titleScene->Draw();
		break;
	case Scene::kGame:
		if (gameScene) gameScene->Draw();
		break;
	case Scene::kGameOver:
		if (gameOverScene) gameOverScene->Draw();
		break;
	case Scene::kClear:
		if (clearScene) clearScene->Draw();
		break;
	}
}