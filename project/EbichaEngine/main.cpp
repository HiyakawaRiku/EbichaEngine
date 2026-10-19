#include "ConvertString.h"
#include <format>

//ファイルに書いたり読んだりするライブラリ
#include <fstream>
//時間を扱うライブラリ
#include <chrono>





#include <wrl.h>


#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include"Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include "D3DResourceLeakChecker.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "Sound.h"
#include "Camera.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"


#include "EMath.h"

using namespace Logger;

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	D3DResourceLeakChecker leakCheck;

	std::unique_ptr<WinApp> winApp = std::make_unique<WinApp>();
	winApp->Initialize();

	std::unique_ptr<DirectXCommon> dxCommon = std::make_unique<DirectXCommon>();
	dxCommon->Initialize(winApp.get());


	// 現在時刻を取得 (UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ログファイルの名前コンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時間 (PCの設定時間) に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	// ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);




	std::unique_ptr<Input> input = std::make_unique<Input>();
	input->Initialize(winApp.get());

	std::unique_ptr<SpriteCommon> spriteCommon = std::make_unique<SpriteCommon>();
	spriteCommon->Initialize(dxCommon.get());

	std::unique_ptr<Object3dCommon> object3dCommon = std::make_unique<Object3dCommon>();
	object3dCommon->Initialize(dxCommon.get());

	std::unique_ptr<ModelCommon> modelCommon = std::make_unique<ModelCommon>();
	modelCommon->Initialize(dxCommon.get());

	std::unique_ptr<SrvManager> srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());

	std::unique_ptr<ImGuiManager> imGuiManager = std::make_unique<ImGuiManager>();
	imGuiManager->Initialize(winApp.get(),dxCommon.get(),srvManager.get());

	// TextureManager の初期化
	TextureManager::GetInstance()->Initialize(dxCommon.get(),srvManager.get());

	ModelManager::GetInstance()->Initialize(dxCommon.get());
	ModelManager::GetInstance()->LoadModel("axis.obj");


	// テクスチャのロード (インデックス 0 が返り、textureDatas_ に追加されます)
	uint32_t textureIndex = TextureManager::GetInstance()->Load("Resources/uvChecker.png");
	uint32_t textureIndex2 = TextureManager::GetInstance()->Load("Resources/monsterBall.png");

	bool useMonsterBall = true;
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
	sprite->Initialize(spriteCommon.get(), (useMonsterBall) ? textureIndex2 : textureIndex);
	std::unique_ptr<Sprite> sprite2 = std::make_unique<Sprite>();
	sprite2->Initialize(spriteCommon.get(), textureIndex2);

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon.get(),"resources","plane.obj");

	std::unique_ptr<Camera> camera = std::make_unique<Camera>();
	camera->SetRotate({ 0.0f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,0.0f,-100.0f });
	object3dCommon->SetDefaultCamera(camera.get());


	std::unique_ptr<Object3d> object3d = std::make_unique<Object3d>();
	object3d->Initialize(object3dCommon.get());
	object3d->SetModel(model.get());

	std::unique_ptr<Object3d> object3d2 = std::make_unique<Object3d>();
	object3d2->Initialize(object3dCommon.get());
	object3d2->SetModel("axis.obj");


	// 2. ParticleManager の初期化 (ここで内部の乱数エンジンも自動初期化されます)
	ParticleManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());

	// 3. パーティクルグループの登録 (テクスチャ読み込み)[cite: 10]
	ParticleManager::GetInstance()->CreateParticleGroup("smoke", "Resources/monsterBall.png");

	// 4. エミッターの生成 (名前: "smoke", 位置: 原点{0,0,0}, 1回に10個放出, 0.2秒間隔)
	std::unique_ptr<ParticleEmitter> emitter =
		std::make_unique<ParticleEmitter>("smoke", Vector3{ 0.0f, 0.0f, 0.0f }, 10, 0.2f);


	std::unique_ptr<Sound> sound= std::make_unique<Sound>();
	sound->Initialize();
	//音声読み込み
	SoundData soundData1 = sound->SoundLoadFile("Resources/fanfare.wav");

	//音声再生
	sound->SoundPlayWave(sound->xAudio2.Get(), soundData1);


	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		if (winApp->ProcessMessage()) {
			break;
		}

		input->Update();

		//数字の0キーが押されていたら
		if (input->TriggerKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n");//出力ウィンドウに「Hit 0」と表示
		}

		//object3d2->SetTranslate({1.0f,1.0f,1.0f});


#ifdef USE_IMGUI
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム所有の処理に置き換える
		ImGui::ShowDemoWindow();

		ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		//ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		//ImGuiの内部コマンドを生成する
		ImGui::Render();
#endif

		// 5. 更新処理
		emitter->Update();                                    // エミッターのタイマー更新＆Emit発火
		ParticleManager::GetInstance()->Update(); // パーティクルの移動・寿命更新

		camera->Update();
		object3d->Update();
		object3d2->Update();

		Vector2 position = sprite->GetPosition();
		position += Vector2{ 0.1f,0.1f };
		sprite->SetPosition(position);

		float rotation = sprite->GetRotation();
		rotation += 0.01f;
		sprite->SetRotation(rotation);

		Vector4 color = sprite->GetColor();
		color.x += 0.01f;
		if (color.x > 1.0f) {
			color.x -= 1.0f;
		}
		sprite->SetColor(color);

		Vector2 size = sprite->GetSize();
		size.x += 0.1f;
		size.y += 0.1f;
		sprite->SetSize(size);

		sprite->SetAnchorPoint({ 0.5f,0.5f });
		//sprite->SetIsFlipX(1);
		//sprite->SetIsFlipY(0);

		sprite->SetTextureSize({ 10.0f, 10.0f });

		useMonsterBall ? sprite->SetTextureIndex(textureIndex2) : sprite->SetTextureIndex(textureIndex);


		sprite->Update();
		//sprite2->Update();

		srvManager->PreDraw();
		dxCommon->PreDraw();



		object3dCommon->CommonDrawSetting();
		object3d->Draw();
		object3d2->Draw();

		ParticleManager::GetInstance()->Draw();

		spriteCommon->CommonDrawSetting();
		sprite->Draw();
		//sprite2->Draw();


		imGuiManager->Draw();


		dxCommon->PostDraw();

	}

	sound->Finalize();

	//XAudio2解放
	sound->xAudio2.Reset();

	//音声データ解放
	sound->SoundUnload(&soundData1);

	imGuiManager->Finalize();

	//signatureBlob->Release();
	//if (errorBlob) {
	//	errorBlob->Release();
	//}
	//pixelShaderBlob->Release();
	//vertexShaderBlob->Release();

	TextureManager::GetInstance()->Finalize();

	object3dCommon->Finalize();
	spriteCommon->Finalize();

	dxCommon->Finalize();
	winApp->Finalize();

	return 0;
}