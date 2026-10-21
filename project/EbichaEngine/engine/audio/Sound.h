#pragma once
#include <cstdint>
//ファイルに書いたり読んだりするライブラリ
#include <fstream>
//時間を扱うライブラリ
#include <chrono>
#include <cassert>

#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <wrl.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

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


class Sound
{
public:
	static Sound* GetInstance();
	void Initialize();
	void Update();
	void Finalize();

	SoundData SoundLoadFile(const std::string& filename);
	// 音声データ解放
	void SoundUnload(SoundData* soundData);
	// 音声再生
	IXAudio2SourceVoice* SoundPlayWave(const SoundData& soundData);


private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
};

