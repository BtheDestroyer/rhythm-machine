#include "audio.h"
#include "sd.h"
#include <cstdint>
#include <array>
#include "pico/stdlib.h"   // stdlib
#include "hardware/irq.h"  // interrupts
#include "hardware/pwm.h"  // pwm
#include "hardware/sync.h" // wait for interrupt
#include "hardware/clocks.h"

static std::array<Audio::buffer, Audio::total_buffer_count> buffers{ { { 0 }, { 0 } } };
static std::size_t empty_buffer_count{ Audio::total_buffer_count };
static std::size_t active_read_buffer{ 0 };
static std::size_t wav_position{ 0 };
static Audio::WAVHeader streaming_file_header;
static std::optional<SDCard::FileReader> streaming_file;

namespace Audio
{
static void pwm_interrupt_handler()
{
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));
    if (!streaming_file.has_value() || wav_position > streaming_file_header.data_size)
    {
        stop_streaming_wave();
        pwm_set_gpio_level(AUDIO_PIN, 0);
        return;
    }
    const Audio::buffer& active_buffer{ buffers[active_read_buffer] };
    // set pwm level
    pwm_set_gpio_level(AUDIO_PIN, active_buffer[wav_position % active_buffer.size()]);
    ++wav_position;
    if (wav_position % active_buffer.size() == 0)
    {
        active_read_buffer ^= 1;
        ++empty_buffer_count;
    }
}

void init()
{
    /* Overclocking for fun but then also so the system clock is a
    * multiple of typical audio sampling rates.
    */
    set_sys_clock_khz(clock_frequency_khz, true);
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    // Setup PWM interrupt to fire when PWM cycle is complete
    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    // set the handle function above
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);

    // Setup PWM for audio output
    pwm_config config = pwm_get_default_config();
    /* Base clock 176,000,000 Hz divide by wrap 250 then the clock divider further divides
    * to set the interrupt rate.
    *
    * 11 KHz is fine for speech. Phone lines generally sample at 8 KHz
    *
    *
    * So clkdiv should be as follows for given sample rate
    *  8.0f for 11 KHz
    *  4.0f for 22 KHz
    *  2.0f for 44 KHz etc
    */
    pwm_config_set_clkdiv(&config, clock_divider);
    pwm_config_set_wrap(&config, 1000);
    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(AUDIO_PIN, 0);
}

static bool validate_wave_header(const WAVHeader& header)
{
    if (std::memcmp(header.magic_riff, "RIFF", 4) != 0)
    {
        return false;
    }
    if (std::memcmp(header.magic_wave, "WAVE", 4) != 0)
    {
        return false;
    }
    if (std::memcmp(header.magic_fmt, "fmt ", 4) != 0)
    {
        return false;
    }
    if (std::memcmp(header.magic_data, "data", 4) != 0)
    {
        return false;
    }
    if (header.format_size != 16)
    {
        return false;
    }
    if (header.bytes_per_frame * header.samples_per_second != header.bytes_per_second)
    {
        return false;
    }
    if (header.bits_per_sample * header.channels / 8 != header.bytes_per_frame)
    {
        return false;
    }
    if (header.format != WAVHeader::Format::PCM)
    {
        return false;
    }
    if (header.channels != 1)
    {
        return false;
    }
    if (header.bits_per_sample != 8)
    {
        return false;
    }
    if (header.samples_per_second != sample_rate)
    {
        return false;
    }
    return true;
}

bool start_streaming_wave(SDCard::FileReader wave_file)
{
    wave_file.read<WAVHeader>(streaming_file_header);
    if (!validate_wave_header(streaming_file_header))
    {
        irq_set_enabled(PWM_IRQ_WRAP, false);
        return false;
    }
    wav_position = 0;
    buffers[0].fill(0);
    buffers[1].fill(0);
    streaming_file = wave_file;
    empty_buffer_count = 2;
    stream_wave_to_inactive_buffer();
    active_read_buffer ^= 1;
    stream_wave_to_inactive_buffer();
    active_read_buffer ^= 1;
    irq_set_enabled(PWM_IRQ_WRAP, true);
    return true;
}

void stream_wave_to_inactive_buffer()
{
    if (!streaming_file.has_value() || empty_buffer_count == 0)
    {
        return;
    }
    Audio::buffer& inactive_buffer{ buffers[active_read_buffer ^ 1] };
    if (wav_position >= streaming_file_header.data_size)
    {
        stop_streaming_wave();
        return;
    }
    streaming_file->read_bytes(std::span{inactive_buffer.begin(), inactive_buffer.end()});
    --empty_buffer_count;
}

void stop_streaming_wave()
{
    streaming_file = std::nullopt;
    irq_set_enabled(PWM_IRQ_WRAP, false);
}
}
