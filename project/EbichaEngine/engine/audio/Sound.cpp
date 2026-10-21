#include "Sound.h"


Sound* Sound::GetInstance()
{
	static Sound instance;
	return &instance;
}

void Sound::Initialize()
{

	IXAudio2MasteringVoice* masterVoice;

	//XAudioエンジンのインスタンスを生成
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);

	hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(hr));

}

void Sound::Finalize()
{

	xAudio2.Reset();

	HRESULT hr = MFShutdown();
	assert(SUCCEEDED(hr));
}

SoundData Sound::SoundLoadFile(const std::string& filename)
{
	HRESULT hr = S_OK;

	// std::string (マルチバイト) から wchar_t* (ワイド文字列) への変換
	wchar_t wfilename[512];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wfilename, filename.c_str(), _countof(wfilename));

	// SourceReader の作成
	Microsoft::WRL::ComPtr<IMFSourceReader> pReader;
	hr = MFCreateSourceReaderFromURL(wfilename, nullptr, &pReader);
	assert(SUCCEEDED(hr));

	// 出力メディアタイプを非圧縮 PCM に設定
	Microsoft::WRL::ComPtr<IMFMediaType> pPartialType;
	hr = MFCreateMediaType(&pPartialType);
	assert(SUCCEEDED(hr));

	hr = pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	assert(SUCCEEDED(hr));

	hr = pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	assert(SUCCEEDED(hr));

	hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPartialType.Get());
	assert(SUCCEEDED(hr));

	// 実際に設定された完全なメディアタイプを取得
	Microsoft::WRL::ComPtr<IMFMediaType> pUncompressedAudioType;
	hr = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pUncompressedAudioType);
	assert(SUCCEEDED(hr));

	// WAVEFORMATEX 構造体の取得
	WAVEFORMATEX* pWfex = nullptr;
	UINT32 cbFormat = 0;
	hr = MFCreateWaveFormatExFromMFMediaType(pUncompressedAudioType.Get(), &pWfex, &cbFormat);
	assert(SUCCEEDED(hr));

	// オーディオデータの全サンプルを一時的に格納するバッファ
	std::vector<BYTE> mediaBuffer;

	while (true)
	{
		DWORD dwFlags = 0;
		Microsoft::WRL::ComPtr<IMFSample> pSample;

		hr = pReader->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0,
			nullptr,
			&dwFlags,
			nullptr,
			&pSample
		);
		assert(SUCCEEDED(hr));

		// ストリームの終わりに達したら読み込み完了
		if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			break;
		}

		if (pSample)
		{
			Microsoft::WRL::ComPtr<IMFMediaBuffer> pBuffer;
			hr = pSample->ConvertToContiguousBuffer(&pBuffer);
			assert(SUCCEEDED(hr));

			BYTE* pAudioData = nullptr;
			DWORD cbCurrentLength = 0;

			hr = pBuffer->Lock(&pAudioData, nullptr, &cbCurrentLength);
			assert(SUCCEEDED(hr));

			// 取得した波形データを追加
			size_t currentSize = mediaBuffer.size();
			mediaBuffer.resize(currentSize + cbCurrentLength);
			memcpy(mediaBuffer.data() + currentSize, pAudioData, cbCurrentLength);

			hr = pBuffer->Unlock();
			assert(SUCCEEDED(hr));
		}
	}

	// 返却用 SoundData の作成
	SoundData soundData = {};
	soundData.wfex = *pWfex;
	soundData.bufferSize = static_cast<unsigned int>(mediaBuffer.size());

	// ヒープメモリを確保してデータをコピー
	soundData.pBuffer = new BYTE[soundData.bufferSize];
	memcpy(soundData.pBuffer, mediaBuffer.data(), soundData.bufferSize);

	// MFCreateWaveFormatExFromMFMediaType で割り当てられたメモリを解放
	CoTaskMemFree(pWfex);

	return soundData;
}


// 音声データ解放
void Sound::SoundUnload(SoundData* soundData)
{
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}


// 音声再生
IXAudio2SourceVoice* Sound::SoundPlayWave(const SoundData& soundData) {
	HRESULT hr;

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	hr = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(hr));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(hr));
	hr = pSourceVoice->Start();
	assert(SUCCEEDED(hr));

	return pSourceVoice; // ★ 生成した SourceVoice を返す
}