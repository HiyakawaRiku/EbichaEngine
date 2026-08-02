#pragma once
#include <wrl.h>
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <string>
#include <vector>
#include <unordered_map>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

// 再生用サウンドデータ構造体
struct SoundData {
	WAVEFORMATEX wfex = {};
	std::vector<BYTE> pBuffer;
	unsigned int bufferSize = 0;
};

class Audio {
public:
	static Audio* GetInstance();

	void Initialize();
	void Finalize();

	// MP3 などの音声ファイルを読み込み
	uint32_t LoadAudioSource(const std::string& filePath);

	// 音声の再生
	void PlayWave(uint32_t soundHandle, bool loop = false, float volume = 1.0f);

	// サウンドデータの解放
	void Unload(uint32_t soundHandle);

private:
	Audio() = default;
	~Audio() = default;
	Audio(const Audio&) = delete;
	Audio& operator=(const Audio&) = delete;

	// Media Foundation を使用して MP3 などのファイルを PCM データにデコード
	bool LoadMediaFile(const std::string& filePath, SoundData& outData);

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;

	// 読み込んだサウンドデータの管理（ハンドル管理）
	std::unordered_map<uint32_t, SoundData> soundDatas_;
	uint32_t nextHandle_ = 1;
};