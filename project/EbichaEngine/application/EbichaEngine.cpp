#include "EbichaEngine.h"
#include "Logger.h"
#include <format>

#include "SceneFactory.h"
#include "Editor.h"

using namespace Logger;

void EbichaEngine::InitializeLog() {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds =
		std::chrono::time_point_cast<std::chrono::seconds>(now);

	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	logStream_.open(logFilePath);
}

void EbichaEngine::Initialize() {
	InitializeLog();

	winApp_ = std::make_unique<WinApp>();
	winApp_->Initialize();

	dxCommon_ = std::make_unique<DirectXCommon>();
	dxCommon_->Initialize(winApp_.get());

	input_ = std::make_unique<Input>();
	input_->Initialize(winApp_.get());

	srvManager_ = std::make_unique<SrvManager>();
	srvManager_->Initialize(dxCommon_.get());

	imGuiManager_ = std::make_unique<ImGuiManager>();
	imGuiManager_->Initialize(winApp_.get(), dxCommon_.get(), srvManager_.get());

	// ゲーム画面を描くテクスチャ（ウィンドウと同じ大きさ）。SRV を使うので SrvManager の後に作る
	gameRenderTexture_ = std::make_unique<RenderTexture>();
	gameRenderTexture_->Initialize(dxCommon_.get(), srvManager_.get(), WinApp::kClientWidth, WinApp::kClientHeight);

	// 各種シングルトンの初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon_.get());
	Object3dCommon::GetInstance()->Initialize(dxCommon_.get());
	ModelCommon::GetInstance()->Initialize(dxCommon_.get());
	TextureManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());
	ModelManager::GetInstance()->Initialize(dxCommon_.get());
	ParticleManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());
	Sound::GetInstance()->Initialize();

	// GamePlayScene の初期化
	SceneManager::GetInstance()->Initialize();
	SceneManager::GetInstance()->SetSceneFactory(std::make_unique<SceneFactory>());
	SceneManager::GetInstance()->ChangeScene("TITLE");

	Editor::GetInstance()->Initialize();
	// エディタの Game ウィンドウに、ゲーム画面のテクスチャを表示してもらう
	Editor::GetInstance()->SetGameTexture(
		gameRenderTexture_->GetSrvGpuHandle().ptr,
		static_cast<float>(gameRenderTexture_->GetWidth()),
		static_cast<float>(gameRenderTexture_->GetHeight()));
}

void EbichaEngine::Run() {
	while (true) {
		if (winApp_->ProcessMessage()) {
			break;
		}

		Update();
		Draw();
	}
}

void EbichaEngine::Update() {
	input_->Update();

#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

#endif


	Editor::GetInstance()->Update();
	SceneManager::GetInstance()->Update(input_.get());

#ifdef USE_IMGUI
	ImGui::Render();
#endif
}

void EbichaEngine::Draw() {
	srvManager_->PreDraw();

#ifdef USE_IMGUI
	// ① ゲームを RenderTexture に描く
	gameRenderTexture_->BeginRender();
	SceneManager::GetInstance()->Draw();
	gameRenderTexture_->EndRender();

	// ② 本物の画面をクリアして、ImGui を描く（Game ウィンドウに①を貼る）
	dxCommon_->PreDraw();
	imGuiManager_->Draw();
#else
	// Release：今までどおり、画面に直接描く
	dxCommon_->PreDraw();
	SceneManager::GetInstance()->Draw();
#endif

	dxCommon_->PostDraw();
}

void EbichaEngine::Finalize() {

	Editor::GetInstance()->Finalize();
	SceneManager::GetInstance()->Finalize();

	Sound::GetInstance()->Finalize();

	imGuiManager_->Finalize();

	ParticleManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();

	Object3dCommon::GetInstance()->Finalize();
	SpriteCommon::GetInstance()->Finalize();

	gameRenderTexture_->Finalize();
	gameRenderTexture_.reset();

	srvManager_->Finalize();

	dxCommon_->Finalize();
	winApp_->Finalize();
}