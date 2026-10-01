#include "Sound.h"

void Sound::Initialize()
{

	//XAudioエンジンのインスタンスを生成
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
}

void Sound::Finalize()
{
	//XAudio2解放
	xAudio2.Reset();
}
