#pragma once
#include <array>
#include <functional>
#include <cstdint>
#include <cmath>
#include "hardware/pio.h"
#include "ws2812.pio.h"

#define LED_IS_RGBW false
#define LED_DATA_PIN 15
#define LED_COUNT 45

struct color {
    std::uint8_t r, g, b;
    color operator*(float f) const
    {
        return {
            static_cast<std::uint8_t>(std::floor(r * f)),
            static_cast<std::uint8_t>(std::floor(g * f)),
            static_cast<std::uint8_t>(std::floor(b * f))
        };
    }
    color& operator*=(float f)
    {
        r = static_cast<std::uint8_t>(std::floor(r * f));
        g = static_cast<std::uint8_t>(std::floor(g * f));
        b = static_cast<std::uint8_t>(std::floor(b * f));
        return *this;
    }
};
namespace colors {
    constexpr static color black{ 0x00, 0x00, 0x00 };
    constexpr static color red{ 0xff, 0x00, 0x00 };
    constexpr static color green{ 0x00, 0xff, 0x00 };
    constexpr static color blue{ 0x00, 0x00, 0xff };
    constexpr static color white{ 0xff, 0xff, 0xff };
}

class LEDs {
public:
    LEDs()
    {
        ws2812_program_init(pio0, 0, pio_add_program(pio0, &ws2812_program), LED_DATA_PIN, 800000, LED_IS_RGBW);
        
        clear();
    }

    void put_pixel(color pixel)
    {
        const std::uint32_t c{
            (static_cast<std::uint32_t>(pixel.r) << 8)
            | (static_cast<std::uint32_t>(pixel.g) << 16)
            | static_cast<std::uint32_t>(pixel.b)
        };
        // if (next_pixel != 0)
        // {
        //     // Pixel 0 is a sacrifical 3.3v->5v level shifter
        //     pio_sm_put_blocking(pio0, 0, 0);
        // }
        // else
        {
            pixels[next_pixel] = pixel;
            pio_sm_put_blocking(pio0, 0, c << 8u);
        }
        next_pixel = (next_pixel + 1) % (LED_COUNT);
    }

    void clear()
    {
        for (std::uint32_t i{ 0 }; i < LED_COUNT; ++i)
        {
            put_pixel(colors::black);
        }
    }

    void pattern_snakes(std::uint32_t t)
    {
        for (std::uint32_t i{ 0 }; i < LED_COUNT; ++i)
        {
            const std::uint32_t x{ (i + t) % 64 };
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
                put_pixel(c);
                continue;
            }
            put_pixel(colors::black);
        }
    }

    const color& get_pixel(std::size_t index) const
    {
        return pixels.at(index);
    }

private:
    std::size_t next_pixel{ 0 };
    std::array<color, LED_COUNT> pixels;
};
