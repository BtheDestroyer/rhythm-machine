#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <span>

#define LED_IS_RGBW false
#define LED_DATA_PIN 15
#define LED_COUNT 45

struct color {
    std::uint8_t r, g, b;
    inline color operator*(float f) const
    {
        return {
            static_cast<std::uint8_t>(std::floor(r * f)),
            static_cast<std::uint8_t>(std::floor(g * f)),
            static_cast<std::uint8_t>(std::floor(b * f))
        };
    }
    inline color& operator*=(float f)
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
    LEDs();

    void put_pixel(color pixel);
    void clear();
    void pattern_snakes(std::uint32_t t);
    void show_pattern(std::span<const color> pattern);
    void show_pattern(std::function<color (std::uint32_t pixel_index)> generator);
    const color& get_pixel(std::size_t index) const;

private:
    std::size_t next_pixel{ 0 };
    std::array<color, LED_COUNT> pixels;
};
