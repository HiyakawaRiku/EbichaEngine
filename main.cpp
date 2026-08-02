#include "EbichaEngine.h"

#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")
#include <fstream>

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの一括初期化
	EbichaEngine* ebichaEngine = EbichaEngine::GetInstance();
	ebichaEngine->Initialize();

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Input* input = Input::GetInstance(); // シングルトンインスタンス

	// ============================
// オーディオ初期化
// ============================
	Audio* audio = Audio::GetInstance();
	audio->Initialize();

	Sprite* sprite = new Sprite;
	sprite->Initialize();

	Sphere* sphere = new Sphere;
	sphere->Initialize();

	Model* modelTeapot = new Model;
	modelTeapot->Initialize("resources", "teapot.obj");
	Model* modelBunny = new Model;
	modelBunny->Initialize("resources", "bunny.obj");
	Model* modelMultiMesh = new Model;
	modelMultiMesh->Initialize("resources", "multiMesh.obj");

	auto normalCamera = std::make_unique<Camera>();
	auto debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize();
	Camera* activeCamera = normalCamera.get();
	bool useDebugCamera = false;

	// CPUで動かす用のTransformを作る
	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	dxCommon->InitializeTexture("resources/uvChecker.png", 1);
	dxCommon->InitializeTexture("resources/monsterBall.png", 2);
	dxCommon->InitializeTexture("resources/sky_sphere.png", 3);
	dxCommon->InitializeTexture("resources/ground_leaf.png", 4);
	dxCommon->InitializeTexture("resources/fence.png", 5);

	// MP3 ファイルのロード (ハンドルが返る)
	uint32_t seHandle = Audio::GetInstance()->LoadAudioSource("Resources/Alarm01.wav");
	uint32_t bgmHandle = Audio::GetInstance()->LoadAudioSource("Resources/420_long_BPM108.mp3");

	// BGMのループ再生開始
	Audio::GetInstance()->PlayWave(bgmHandle, true, 0.5f);

	MSG msg{};
	// ウィンドウのxボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			input->Update();

			normalCamera->Update();

			if (activeCamera != normalCamera.get()) {
				activeCamera->Update();
			}
			dxCommon->PreDraw();

			// オブジェクトの更新
			sprite->Update(activeCamera);
			sphere->Update(activeCamera);
			modelTeapot->Update(activeCamera);
			modelBunny->Update(activeCamera);
			modelMultiMesh->Update(activeCamera);

			// 床のグリッド
			DebugRenderer::AddGrid(20.0f, 20, { 0.5f, 0.5f, 0.5f, 1.0f });

			if (input->PushKey(DIK_P)) {
				activeCamera->transform_.rotate.x -= 0.03f;
			}

			if (input->PushButton(XINPUT_GAMEPAD_DPAD_RIGHT)) {
				activeCamera->transform_.rotate.x += 0.03f;
			}

#ifdef _DEBUG

			ImGui::Begin("Settings");


			ImGui::End();

#endif


			if (useDebugCamera) {
				debugCamera->DrawFrustum(normalCamera.get());
			}

			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			sprite->materialData->uvTransform = uvTransformMatrix;

			sprite->Draw(1);
			sphere->Draw(1);
			modelTeapot->Draw(1);
			modelBunny->Draw(1);
			modelMultiMesh->Draw(1);

			DebugRenderer::Flush(activeCamera);

			dxCommon->PostDraw();

		}
	}

	// ============================
	// 後始末
	// ============================

	// --- サウンド解放 ---
	audio->Unload(bgmHandle);
	audio->Unload(seHandle);

	// --- オーディオ終了 ---
	audio->Finalize();

	DebugRenderer::Finalize();
	ebichaEngine->Finalize();

	return 0;
}