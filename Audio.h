#pragma once
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")
#include <fstream>
#include <string>
#include <cstdint>

/// <summary>
/// 音声データ構造体
/// </summary>
struct SoundData {
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int bufferSize;
};

/// <summary>
/// 音声管理クラス
/// </summary>
class Audio {
public:
	/// <summary>
	/// シングルトンインスタンス取得
	/// </summary>
	static Audio* GetInstance();

	/// <summary>
	/// XAudio2初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// WAVファイル読み込み
	/// </summary>
	SoundData LoadWave(const char* filename);

	/// <summary>
	/// 音声再生
	/// </summary>
	void PlayWave(const SoundData& soundData);

	/// <summary>
	/// 音声データ解放
	/// </summary>
	void Unload(SoundData* soundData);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

private:
	Audio() = default;
	~Audio() = default;
	Audio(const Audio&) = delete;
	Audio& operator=(const Audio&) = delete;

	IXAudio2* xAudio2_ = nullptr;
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;
};