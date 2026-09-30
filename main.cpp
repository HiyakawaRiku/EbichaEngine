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


#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")



#include "EMath.h"
#include "Input.h"
#include "EWindow.h"
#include "EDirectX.h"
#include "D3DResourceLeakChecker.h"
#include "Sprite.h"
#include "Object3d.h"


#include "Logger.h"
using namespace Logger;



// チャンクヘッダ
struct ChunkHeader
{
	char id[4]; // チャンク毎のID
	int32_t size; // チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader
{
	ChunkHeader chunk; // "RIFF"
	char type[4]; // "WAVE"
};

// FMTチャンク
struct FormatChunk
{
	ChunkHeader chunk; // "fmt "
	WAVEFORMATEX fmt; // 波形フォーマット
};

// 音声データ
struct SoundData
{
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};

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




SoundData SoundLoadWave(const char* filename)
{
	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());


	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Waveファイルを閉じる
	file.close();

	// returnする為の音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}


// 音声データ解放
void SoundUnload(SoundData* soundData)
{
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}


// 音声再生
void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {

	HRESULT hr;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	hr = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(hr));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	hr = pSourceVoice->Start();
}


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



	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = eDirectX->CreateTextureResource(metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = eDirectX->UploadTextureData(textureResource, mipImages);


	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture(object3d->modelData.material.textureFilePath);
	//DirectX::ScratchImage mipImages2 = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = eDirectX->CreateTextureResource(metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = eDirectX->UploadTextureData(textureResource2, mipImages2);

	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages3 = LoadTexture("resources/circle.png");
	const DirectX::TexMetadata& metadata3 = mipImages3.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource3 = eDirectX->CreateTextureResource(metadata3);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource3 = eDirectX->UploadTextureData(textureResource3, mipImages3);


	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc3{};
	srvDesc3.Format = metadata3.format;
	srvDesc3.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc3.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc3.Texture2D.MipLevels = UINT(metadata3.mipLevels);

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
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = eDirectX->GetSRVCPUDescriptorHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = eDirectX->GetSRVGPUDescriptorHandle(1);

	// SRVの生成
	eDirectX->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = eDirectX->GetSRVCPUDescriptorHandle(2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = eDirectX->GetSRVGPUDescriptorHandle(2);

	// SRVの生成
	eDirectX->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU3 = eDirectX->GetSRVCPUDescriptorHandle(4);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU3 = eDirectX->GetSRVGPUDescriptorHandle(4);

	// SRVの生成
	eDirectX->GetDevice()->CreateShaderResourceView(textureResource3.Get(), &srvDesc3, textureSrvHandleCPU3);


	std::unique_ptr<Sprite>sprite = std::make_unique<Sprite>();
	sprite->Initialize(eDirectX.get());




	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	//XAudioエンジンのインスタンスを生成
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);

	//音声読み込み
	SoundData soundData1 = SoundLoadWave("Resources/fanfare.wav");

	//音声再生
	SoundPlayWave(xAudio2.Get(), soundData1);


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

		object3d->Draw(eDirectX.get(),textureSrvHandleGPU,textureSrvHandleGPU2,instancingSrvHandleGPU);

		sprite->Draw(eDirectX.get(),textureSrvHandleGPU);

		// 実際のcommandListのImGuiの描画コマンドを積む
#ifdef USE_IMGUI
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), eDirectX->GetCommandList());
#endif

		eDirectX->PostDraw();

	}

	//XAudio2解放
	xAudio2.Reset();

	//音声データ解放
	SoundUnload(&soundData1);

	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	// こういうもんである。初期化と逆順に行う
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	//object3d->Finalize();
	//sprite->Finalize();

	eWindow->Finalize();

	return 0;
}