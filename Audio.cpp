#include "Audio.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>

namespace {
struct ChunkHeader { char id[4]; int32_t size; };
struct RiffHeader { ChunkHeader chunk; char type[4]; };
}

void Audio::Initialize() {
	assert(SUCCEEDED(XAudio2Create(&xaudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR)));
	assert(SUCCEEDED(xaudio2_->CreateMasteringVoice(&masterVoice_)));
}

void Audio::LoadAndPlayWave(const char* filename) {
	std::ifstream file(filename, std::ios::binary);
	assert(file.is_open());
	RiffHeader riff{};
	file.read(reinterpret_cast<char*>(&riff), sizeof(riff));
	assert(std::strncmp(riff.chunk.id, "RIFF", 4) == 0 && std::strncmp(riff.type, "WAVE", 4) == 0);
	ChunkHeader chunk{};
	while (file.read(reinterpret_cast<char*>(&chunk), sizeof(chunk))) {
		if (std::strncmp(chunk.id, "fmt ", 4) == 0) {
			assert(chunk.size <= sizeof(format_));
			file.read(reinterpret_cast<char*>(&format_), chunk.size);
		} else if (std::strncmp(chunk.id, "data", 4) == 0) {
			buffer_.resize(chunk.size);
			file.read(reinterpret_cast<char*>(buffer_.data()), chunk.size);
			break;
		} else {
			file.seekg(chunk.size, std::ios::cur);
		}
	}
	assert(!buffer_.empty());
	assert(SUCCEEDED(xaudio2_->CreateSourceVoice(&sourceVoice_, &format_)));
	XAUDIO2_BUFFER buffer{};
	buffer.pAudioData = buffer_.data();
	buffer.AudioBytes = static_cast<UINT32>(buffer_.size());
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	assert(SUCCEEDED(sourceVoice_->SubmitSourceBuffer(&buffer)));
	assert(SUCCEEDED(sourceVoice_->Start()));
}

void Audio::Finalize() {
	if (sourceVoice_) { sourceVoice_->DestroyVoice(); sourceVoice_ = nullptr; }
	if (masterVoice_) { masterVoice_->DestroyVoice(); masterVoice_ = nullptr; }
	buffer_.clear();
	xaudio2_.Reset();
}
