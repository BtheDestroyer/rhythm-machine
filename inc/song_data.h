#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include "leds.h"
#include "sd.h"

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
    float speed{ 1.0f }; // TODO
    std::array<std::uint8_t, 2> padding; // Unused

    [[nodiscard]] color get_pixel_color(bool bright = true) const;
} __attribute__((packed));
static_assert(sizeof(Note) == 16);

using note_list = std::vector<Note>;
struct Song 
{
    constexpr static std::uint16_t verison_major{ 1 };

    struct Header
    {
        char magic_note[4]; // Should always be "NOTE"
        std::uint16_t version_major; // Should match song_data::Song::verison_major
        std::uint16_t version_minor; // Indicates minor changes which should be backwards compatible
        std::uint32_t ms_per_pixel;
        std::size_t note_count;
        std::array<char, 32> author;
        std::uint8_t difficulty; // 1-10
        std::array<std::uint8_t, 31> padding; // Unused

        bool validate() const;
    } __attribute__((packed));
    Song() = default;
    Song(Song&&) = default;
    Song(const Song&) = default;
    Song(const Header& header)
        : header{ header }, notes{header.note_count}
    {}
    Song& operator=(Song&&) = default;
    Song& operator=(const Song&) = default;

    Header header;
    note_list notes;
    std::uint32_t current_time_ms{ 0 };

    static std::optional<Song> load_from_note_file(SDCard::FileReader file);

    [[nodiscard]] std::array<color, visible_led_count> render_leds() const;

    [[nodiscard]] std::int64_t note_length_to_pixel_count(std::uint32_t time_ms) const;
    [[nodiscard]] std::int64_t pixel_count_to_note_length(std::uint32_t count) const;
    [[nodiscard]] std::int64_t note_time_to_pixel_index(std::uint32_t time_ms, Note::Direction direction) const;
    [[nodiscard]] std::int64_t pixel_index_to_note_time(std::size_t index, Note::Direction direction) const;
};
}
