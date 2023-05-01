#pragma once
#include <cstdint>
#include "sd.h"
#define AUDIO_PIN 16

namespace Audio
{
constexpr std::size_t clock_frequency_khz{ 176'000 };
constexpr std::size_t audio_buffer_size{ 2048 };

constexpr float clock_divider{ 8.0f };
constexpr std::size_t sample_rate{ static_cast<std::size_t>(
    static_cast<float>(clock_frequency_khz) / clock_divider
)};

using buffer = std::array<std::uint8_t, audio_buffer_size>;
constexpr std::size_t total_buffer_count{ 2 };

struct WAVHeader
{
    char magic_riff[4]; // Should always be "RIFF"
    std::uint32_t file_size;
    char magic_wave[4]; // Should always be "WAVE"
    char magic_fmt[4]; // Should always be "fmt "
    std::uint32_t format_size; // Should always be 16
    enum class Format : std::uint16_t {
        PCM = 1
    } format;
    std::uint16_t channels;
    std::uint32_t samples_per_second;
    std::uint32_t bytes_per_second; // Sample Rate * Bits per sample * Channels / 8
    std::uint16_t bytes_per_frame; // Bits per sample * Channels / 8
    std::uint16_t bits_per_sample;
    char magic_data[4]; // Should always be "data"
    std::uint32_t data_size;
};

void init();
bool start_streaming_wave(SDCard::FileReader wave_file);
void stream_wave_to_inactive_buffer();
void stop_streaming_wave();
}
