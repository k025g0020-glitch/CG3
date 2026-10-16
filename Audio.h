#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <vector>

class Audio {
public:
	void Initialize();
	void LoadAndPlayWave(const char* filename);
	void Finalize();
private:
	Microsoft::WRL::ComPtr<IXAudio2> xaudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;
	IXAudio2SourceVoice* sourceVoice_ = nullptr;
	WAVEFORMATEX format_{};
	std::vector<BYTE> buffer_;
};
