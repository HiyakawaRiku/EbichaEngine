#include "Audio.h"
#include <cassert>
#include <system_error>

Audio* Audio::GetInstance() {
	static Audio instance;
	return &instance;
}

void Audio::Initialize() {
	HRESULT result;

	// Media Foundation の初期化
	result = MFStartup(MF_VERSION, MFSTARTUP_FULL);
	assert(SUCCEEDED(result));

	// XAudio2 エンジンのインスタンス生成
	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスの生成
	result = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result));
}

void Audio::Finalize() {
	// 1. 全ての再生中ボイスを停止して破棄（最優先で行う）
	for (auto& [handle, voiceSet] : activeVoices_) {
		for (auto* voice : voiceSet) {
			if (voice) {
				voice->Stop();
				voice->FlushSourceBuffers();
				voice->DestroyVoice();
			}
		}
	}
	activeVoices_.clear();

	// 2. ボイス停止後に初めてサウンドデータ（波形メモリ）をクリアする
	soundDatas_.clear();

	// 3. マスターボイスの破棄
	if (masterVoice_) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}

	// 4. XAudio2 本体の解放
	xAudio2_.Reset();

	// 5. Media Foundation の終了処理
	MFShutdown();
}

bool Audio::LoadMediaFile(const std::string& filePath, SoundData& outData) {
	HRESULT result;

	wchar_t wFilePath[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, filePath.c_str(), -1, wFilePath, _countof(wFilePath));

	Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
	result = MFCreateSourceReaderFromURL(wFilePath, nullptr, &sourceReader);
	if (FAILED(result)) return false;

	Microsoft::WRL::ComPtr<IMFMediaType> partialType;
	result = MFCreateMediaType(&partialType);
	if (FAILED(result)) return false;

	result = partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	result = partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	result = sourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, partialType.Get());
	if (FAILED(result)) return false;

	Microsoft::WRL::ComPtr<IMFMediaType> uncompressedAudioType;
	result = sourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &uncompressedAudioType);
	if (FAILED(result)) return false;

	WAVEFORMATEX* pWfex = nullptr;
	UINT32 cbFormat = 0;
	result = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType.Get(), &pWfex, &cbFormat);
	if (FAILED(result)) return false;

	outData.wfex = *pWfex;
	CoTaskMemFree(pWfex);

	std::vector<BYTE> audioData;
	while (true) {
		DWORD flags = 0;
		Microsoft::WRL::ComPtr<IMFSample> sample;
		result = sourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, &sample);
		if (FAILED(result) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) {
			break;
		}

		if (sample) {
			Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
			result = sample->ConvertToContiguousBuffer(&mediaBuffer);
			if (SUCCEEDED(result)) {
				BYTE* pAudioBytes = nullptr;
				DWORD cbCurrentLength = 0;
				result = mediaBuffer->Lock(&pAudioBytes, nullptr, &cbCurrentLength);
				if (SUCCEEDED(result)) {
					audioData.insert(audioData.end(), pAudioBytes, pAudioBytes + cbCurrentLength);
					mediaBuffer->Unlock();
				}
			}
		}
	}

	outData.pBuffer = std::move(audioData);
	outData.bufferSize = static_cast<unsigned int>(outData.pBuffer.size());

	return true;
}

uint32_t Audio::LoadAudioSource(const std::string& filePath) {
	SoundData data;
	if (!LoadMediaFile(filePath, data)) {
		assert(false && "Failed to load audio file!");
		return 0;
	}

	uint32_t handle = nextHandle_++;
	soundDatas_[handle] = std::move(data);
	return handle;
}

void Audio::PlayWave(uint32_t soundHandle, bool loop, float volume) {
	auto it = soundDatas_.find(soundHandle);
	if (it == soundDatas_.end()) return;

	const SoundData& soundData = it->second;

	// SourceVoice の作成
	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT result = xAudio2_->CreateSourceVoice(&sourceVoice, &soundData.wfex);
	if (FAILED(result)) return;

	// 作成したボイスを追跡対象に登録
	activeVoices_[soundHandle].insert(sourceVoice);

	// 音量設定
	sourceVoice->SetVolume(volume);

	// バッファの設定
	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = soundData.pBuffer.data();
	buffer.AudioBytes = soundData.bufferSize;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

	// 再生開始
	sourceVoice->SubmitSourceBuffer(&buffer);
	sourceVoice->Start(0);
}

void Audio::Unload(uint32_t soundHandle) {
	// 該当ハンドルのボイスが存在すれば停止して破棄
	auto it = activeVoices_.find(soundHandle);
	if (it != activeVoices_.end()) {
		for (auto* voice : it->second) {
			if (voice) {
				voice->Stop();
				voice->FlushSourceBuffers();
				voice->DestroyVoice();
			}
		}
		activeVoices_.erase(it);
	}

	// ボイスを破棄した後にサウンドデータを解放
	soundDatas_.erase(soundHandle);
}