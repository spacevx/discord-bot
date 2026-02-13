#pragma once

#include <whisper.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace bot {

class VoiceTrigger {
public:
    using trigger_callback_t = std::function<void(uint64_t guild_id)>;

    static VoiceTrigger& instance() {
        static VoiceTrigger vt;
        return vt;
    }

    bool init(const std::string& model_path, trigger_callback_t on_trigger) {
        on_trigger_ = std::move(on_trigger);
        whisper_context_params cparams = whisper_context_default_params();
        ctx_ = whisper_init_from_file_with_params(model_path.c_str(), cparams);
        return ctx_ != nullptr;
    }

    ~VoiceTrigger() {
        if (ctx_) whisper_free(ctx_);
    }

    void feed_audio(uint64_t guild_id, const uint8_t* pcm_data, size_t byte_count) {
        if (!ctx_) return;

        const auto* samples = reinterpret_cast<const int16_t*>(pcm_data);
        size_t stereo_samples = byte_count / 4;

        std::lock_guard lock(mutex_);
        auto& buf = buffers_[guild_id];

        for (size_t i = 0; i < stereo_samples; i += 3) {
            float sample = (samples[i * 2] + samples[i * 2 + 1]) / (2.0f * 32768.0f);
            buf.push_back(sample);
        }

        if (buf.size() >= 48000 && !inferring_.test_and_set()) {
            auto audio = std::move(buf);
            buf.clear();

            std::thread([this, guild_id, audio = std::move(audio)]() {
                run_inference(guild_id, audio);
            }).detach();
        }
    }

    void clear_buffer(uint64_t guild_id) {
        std::lock_guard lock(mutex_);
        buffers_.erase(guild_id);
    }

private:
    VoiceTrigger() { inferring_.clear(); }

    void run_inference(uint64_t guild_id, const std::vector<float>& audio) {
        whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        params.language = "fr";
        params.n_threads = 2;
        params.no_timestamps = true;
        params.print_progress = false;
        params.print_realtime = false;
        params.print_special = false;
        params.print_timestamps = false;

        if (whisper_full(ctx_, params, audio.data(), static_cast<int>(audio.size())) != 0) {
            inferring_.clear();
            return;
        }

        int n_segments = whisper_full_n_segments(ctx_);
        std::string text;
        for (int i = 0; i < n_segments; ++i) {
            text += whisper_full_get_segment_text(ctx_, i);
        }

        inferring_.clear();

        std::transform(text.begin(), text.end(), text.begin(),
            [](unsigned char c) { return std::tolower(c); });

        if (text.find("delta charlie") != std::string::npos ||
            text.find("code delta") != std::string::npos ||
            text.find("x05") != std::string::npos ||
            text.find("x 05") != std::string::npos) {
            if (on_trigger_) on_trigger_(guild_id);
        }
    }

    whisper_context* ctx_{nullptr};
    std::mutex mutex_;
    std::unordered_map<uint64_t, std::vector<float>> buffers_;
    std::atomic_flag inferring_{};
    trigger_callback_t on_trigger_;
};

} // namespace bot
