#include "ConvertString.h"
#include <format>

//ファイルに書いたり読んだりするライブラリ
#include <fstream>
//時間を扱うライブラリ
#include <chrono>



#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#include <wrl.h>



#include "EMath.h"
#include "Input.h"
#include "EWindow.h"
#include "EDirectX.h"
#include "D3DResourceLeakChecker.h"
#include "Sprite.h"
#include "Object3d.h"

#include "Sound.h"
#include "TextureManager.h"


#include "Logger.h"
using namespace Logger;




enum BlendMode {
	//!< ブレンドなし
	kBlendModeNone,
	//!< 通常αブレンド。デフォルト。 Src * SrcA + Dest * (1 - SrcA)
	kBlendModeNormal,
	//!< 加算。Src * SrcA + Dest * 1
	kBlendModeAdd,
	//!< 減算。Dest * 1 - Src * SrcA
	kBlendModeSubtract,
	//!< 乗算。Src * 0 + Dest * Src
	kBlendModeMultily,
	//!< スクリーン。Src * (1 - Dest) + Dest * 1
	kBlendModeScreen,
	// 利用してはいけない
	kCountOfBlendMode,
};


//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	D3DResourceLeakChecker leakCheck;

	std::unique_ptr<EWindow> eWindow = std::make_unique<EWindow>();
	eWindow->Initialize();


	std::unique_ptr<EDirectX>eDirectX = std::make_unique<EDirectX>();
	eDirectX->Initialize(eWindow.get());


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
	input->Initialize(eWindow.get());




	std::unique_ptr<Object3d> object3d = std::make_unique<Object3d>();
	object3d->Initialize(eDirectX.get());

	//Transform関数を作る

	std::unique_ptr<TextureManager> textureManager = std::make_unique<TextureManager>();
	textureManager->Initialize(eDirectX.get());


	// Textureを読んで転送する
	DirectX::ScratchImage mipImages;
	DirectX::TexMetadata metadata;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	textureManager->LoadTexture("resources/uvChecker.png", mipImages, textureResource, intermediateResource, metadata);


	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2;
	DirectX::TexMetadata metadata2;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2;
	textureManager->LoadTexture(object3d->modelData.material.textureFilePath, mipImages2, textureResource2, intermediateResource2, metadata2);


	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages3;
	DirectX::TexMetadata metadata3;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource3;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource3;
	textureManager->LoadTexture("resources/circle.png", mipImages3, textureResource3, intermediateResource3, metadata3);


	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = object3d->kNumMaxInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = eDirectX->GetSRVCPUDescriptorHandle(3);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = eDirectX->GetSRVGPUDescriptorHandle(3);
	eDirectX->GetDevice()->CreateShaderResourceView(object3d->instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);


	// SRVを作成するDescriptorHeapの場所を決める
	// 先頭はImGuiが使っているのでその次を使う
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;
	textureManager->CreateSRV(metadata, textureResource, 1, textureSrvHandleCPU, textureSrvHandleGPU);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2;
	textureManager->CreateSRV(metadata2, textureResource2, 2, textureSrvHandleCPU2, textureSrvHandleGPU2);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU3;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU3;
	textureManager->CreateSRV(metadata3, textureResource3, 4, textureSrvHandleCPU3, textureSrvHandleGPU3);

	std::unique_ptr<Sprite>sprite = std::make_unique<Sprite>();
	sprite->Initialize(eDirectX.get());


	std::unique_ptr<Sound> sound = std::make_unique<Sound>();
	sound->Initialize();


	//音声読み込み
	SoundData soundData1 = SoundLoadWave("Resources/fanfare.wav");

	//音声再生
	SoundPlayWave(sound->xAudio2.Get(), soundData1);


	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (eWindow->ProcessMessage()) {
			break;
		}


		input->Update();

		//数字の0キーが押されていたら
		if (input->TriggerKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n");//出力ウィンドウに「Hit 0」と表示
		}

		for (std::list<Particle>::iterator particleIterator = object3d->particles.begin(); particleIterator != object3d->particles.end(); ++particleIterator) {
			(*particleIterator).transform.translate.x += (*particleIterator).velocity.x * kDeltaTime;
			(*particleIterator).transform.translate.y += (*particleIterator).velocity.y * kDeltaTime;
			(*particleIterator).transform.translate.z += (*particleIterator).velocity.z * kDeltaTime;
		}


#ifdef USE_IMGUI
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム所有の処理に置き換える
		ImGui::ShowDemoWindow();

		ImGui::Checkbox("useMonsterBall", &object3d->useMonsterBall);

		//ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		//ImGui::DragFloat("intensity", &directionalLightData->intensity, 0.01f, 0.0f, 10.0f);

		if (ImGui::Button("Add Particle")) {
			object3d->particles.splice(object3d->particles.end(), Emit(object3d->emitter, randomEngine));
		}

		ImGui::DragFloat3("EmitterTranslate", &object3d->emitter.transform.translate.x, 0.01f, -100.0f, 100.0f);

		//ImGuiの内部コマンドを生成する
		ImGui::Render();
#endif


		sprite->Update();
		object3d->Update();

		eDirectX->PreDraw();

		object3d->Draw(eDirectX.get(), textureSrvHandleGPU, textureSrvHandleGPU2, instancingSrvHandleGPU);

		sprite->Draw(eDirectX.get(), textureSrvHandleGPU);

		// 実際のcommandListのImGuiの描画コマンドを積む
#ifdef USE_IMGUI
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), eDirectX->GetCommandList());
#endif

		eDirectX->PostDraw();

	}

	sound->Finalize();

	//音声データ解放
	SoundUnload(&soundData1);

	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	// こういうもんである。初期化と逆順に行う
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	object3d->Finalize();
	sprite->Finalize();

	eWindow->Finalize();

	return 0;
}