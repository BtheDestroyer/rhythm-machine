#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <span>

#define LED_IS_RGBW false
#define LED_DATA_PIN 15
constexpr std::size_t led_count{ 45 };
constexpr bool using_sacrificial_led{ true };
constexpr std::size_t visible_led_count{ led_count - (using_sacrificial_led ? 1 : 0) };

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
        return *this = *this * f;
    }
    inline color operator+(const color& c) const
    {
        return {
            static_cast<std::uint8_t>(std::clamp(r + c.r, 0, 255)),
            static_cast<std::uint8_t>(std::clamp(g + c.g, 0, 255)),
            static_cast<std::uint8_t>(std::clamp(b + c.b, 0, 255))
        };
    }
    inline color& operator+=(const color& c)
    {
        return *this = *this + c;
    }
};
namespace colors {
    constexpr color black{ 0x00, 0x00, 0x00 };
    constexpr color red{ 0xff, 0x00, 0x00 };
    constexpr color green{ 0x00, 0xff, 0x00 };
    constexpr color blue{ 0x00, 0x00, 0xff };
    constexpr color white{ 0xff, 0xff, 0xff };
}

class LEDs {
public:
    LEDs();

    void put_pixel(color pixel);
    void clear();
    void pattern_snakes(std::uint32_t t);
    void show_pattern(std::span<const color> pattern);
    void show_pattern(const std::array<color, led_count>& pattern);
    void show_pattern(const std::array<color, visible_led_count>& pattern);
    void show_pattern(std::function<color (std::uint32_t pixel_index)> generator);
    const color& get_pixel(std::size_t index) const;

private:
    std::size_t next_pixel{ 0 };
    std::array<color, led_count> pixels;
};
