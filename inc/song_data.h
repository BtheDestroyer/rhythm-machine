#pragma once
#include <cstdint>
#include <vector>
#include "leds.h"

namespace song_data
{
struct Note
{
    enum Color : std::uint8_t {
        Red,
        Green,
        Blue
    } note_color;
    enum Direction : std::uint8_t {
        Clockwise,
        Counterclockwise,
        Right = Clockwise,
        Left = Counterclockwise
    } direction;
    std::uint32_t start_ms;
    std::uint32_t length_ms;

    [[nodiscard]] color get_pixel_color(bool bright = true) const;
};

using note_list = std::vector<Note>;
struct Song 
{
    note_list notes;
    std::uint32_t current_time_ms{ 0 };
    std::uint32_t ms_per_pixel{ 16 };

    [[nodiscard]] std::array<color, visible_led_count> render_leds() const;

    [[nodiscard]] std::int64_t note_length_to_pixel_count(std::uint32_t time_ms) const;
    [[nodiscard]] std::int64_t pixel_count_to_note_length(std::uint32_t count) const;
    [[nodiscard]] std::int64_t note_time_to_pixel_index(std::uint32_t time_ms, Note::Direction direction) const;
    [[nodiscard]] std::int64_t pixel_index_to_note_time(std::size_t index, Note::Direction direction) const;
};
}
