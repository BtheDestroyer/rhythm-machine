#include "leds.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

LEDs::LEDs()
{
    ws2812_program_init(pio0, 0, pio_add_program(pio0, &ws2812_program), LED_DATA_PIN, 800000, LED_IS_RGBW);

    clear();
}

void LEDs::put_pixel(color pixel)
{
    const std::uint32_t c{
        (static_cast<std::uint32_t>(pixel.r) << 8) | (static_cast<std::uint32_t>(pixel.g) << 16) | static_cast<std::uint32_t>(pixel.b)};
    pixels[next_pixel] = pixel;
    pio_sm_put_blocking(pio0, 0, c << 8u);
    next_pixel = (next_pixel + 1) % (LED_COUNT);
}

void LEDs::clear()
{
    show_pattern([]([[maybe_unused]] std::uint32_t x){ return colors::black; });
}

void LEDs::pattern_snakes(std::uint32_t t)
{
    show_pattern([&t](std::uint32_t i){
        const std::uint32_t x{(i + t) % 64};
        if (x < 10)
        {
            color c;
            switch (((i + t) % (64 * 3)) / 64)
            {
            case 0:
                c = colors::red;
                break;
            case 1:
                c = colors::green;
                break;
            case 2:
                c = colors::blue;
                break;
            }
            if (x != 0)
            {
                c *= 0.1f;
            }
            return c;
        }
        return colors::black;
    });
}

void LEDs::show_pattern(std::span<const color> pattern)
{
    for (std::uint32_t i{0}; i < LED_COUNT && i < pattern.size(); ++i)
    {
        put_pixel(pattern[i]);
    }
}

void LEDs::show_pattern(std::function<color (std::uint32_t pixel_index)> generator)
{
    for (std::uint32_t i{0}; i < LED_COUNT; ++i)
    {
        put_pixel(std::invoke(generator, i));
    }
}


const color &LEDs::get_pixel(std::size_t index) const
{
    return pixels.at(index);
}
