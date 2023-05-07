#include "song_data.h"
#include <algorithm>
#include <cstring>

namespace song_data
{
    color Note::get_pixel_color(bool bright /* = true */) const
    {
        const float brightness{ bright ? 1.0f : 0.1f };
        switch (note_color)
        {
        case Color::Red:
            return colors::red * brightness;
        case Color::Green:
            return colors::green * brightness;
        case Color::Blue:
            return colors::blue * brightness;
        }
        return colors::black;
    }

    bool Song::Header::validate() const
    {
        if (std::memcmp(magic_note, "NOTE", 4) != 0)
        {
            return false;
        }
        if (version_major != Song::verison_major)
        {
            return false;
        }
        if (difficulty < 1 || difficulty > 10)
        {
            return false;
        }
        return true;
    }

    std::optional<Song> Song::load_from_note_file(SDCard::FileReader file)
    {
        Header header;
        if (!file.read(header) || !header.validate())
        {
            return std::nullopt;
        }
        Song song{ header };
        for (Note& note : song.notes)
        {
            if (!file.read(note))
            {
                return std::nullopt;
            }
        }
        return song;
    }

    std::array<color, visible_led_count> Song::render_leds() const
    {
        std::array<color, visible_led_count> leds{ colors::black };
        // TODO: Figure out this math to improve efficiency of note lookup
        // const auto first_shown_note{
        //     std::lower_bound(
        //         notes.begin(), notes.end(),
        //         Note{ .start_ms = current_time_ms, .end_ms = current_time_ms },
        //         [](const Note& lhs, const Note& rhs) { return lhs.start_ms > rhs.start_ms || lhs.end_ms <= rhs.end_ms; }
        //     )
        // };
        // const auto last_shown_note{
        //     std::upper_bound(
        //         notes.begin(), notes.end(),
        //         Note{ .start_ms = current_time_ms, .end_ms = current_time_ms },
        //         [](const Note& lhs, const Note& rhs) { return lhs.start_ms > rhs.start_ms || lhs.end_ms <= rhs.end_ms; }
        //     )
        // };

        //for (auto Note{ first_shown_note }; note != last_shown_note; ++note)
        for (const Note& note : notes)
        {
            if (note.start_ms - pixel_count_to_note_length(visible_led_count) > current_time_ms)
            {
                continue;
            }
            const std::int64_t length_leds{ note_length_to_pixel_count(note.length_ms) };
            // convert start and end times to pixel indices
            const std::int64_t start_index{ note_time_to_pixel_index(note.start_ms, note.direction) };
            if (start_index >= 0 && start_index < visible_led_count)
            {
                leds[start_index] += note.get_pixel_color(true);
            }
            switch (note.direction)
            {
                case Note::Direction::Counterclockwise:
                {
                    if (start_index + length_leds < 0)
                    {
                        break;
                    }
                    const std::int64_t end_index{ std::clamp<std::int64_t>(
                        start_index - length_leds,
                        0, visible_led_count
                    ) };
                    // fill leds between pixel indices
                    for (std::size_t index{ static_cast<std::size_t>(end_index) }; index < static_cast<std::size_t>(start_index) && index < visible_led_count; ++index)
                    {
                        leds[index] += note.get_pixel_color(false);
                    }
                    break;
                }
                case Note::Direction::Clockwise:
                {
                    if (start_index + length_leds < 0)
                    {
                        break;
                    }
                    const std::int64_t end_index{ std::clamp<std::int64_t>(
                        start_index + length_leds,
                        0, visible_led_count - 1
                    ) };
                    // fill leds between pixel indices
                    for (std::size_t index{ static_cast<std::size_t>(std::max(start_index + 1ll, 0ll)) }; index <= static_cast<std::size_t>(end_index); ++index)
                    {
                        leds[index] += note.get_pixel_color(false);
                    }
                    break;
                }
            }
        }
        return leds;
    }

    // TODO: Account for note.speed
    std::int64_t Song::note_length_to_pixel_count(std::uint32_t time_ms) const
    {
        return static_cast<int64_t>(time_ms) / static_cast<int64_t>(header.ms_per_pixel);
    }

    // TODO: Account for note.speed
    std::int64_t Song::pixel_count_to_note_length(std::uint32_t count) const
    {
        return static_cast<int64_t>(count) * static_cast<int64_t>(header.ms_per_pixel);
    }

    // TODO: Account for note.speed
    std::int64_t Song::note_time_to_pixel_index(std::uint32_t time_ms, Note::Direction direction) const
    {
        const std::int64_t offset{ (static_cast<std::int64_t>(current_time_ms) - static_cast<std::int64_t>(time_ms)) / static_cast<std::int64_t>(header.ms_per_pixel) };
        if (direction == Note::Direction::Counterclockwise)
        {
            return visible_led_count + offset;
        }
        return -offset;
    }

    // TODO: Account for note.speed
    std::int64_t Song::pixel_index_to_note_time(std::size_t index, Note::Direction direction) const
    {
        if (direction == Note::Direction::Counterclockwise)
        {
            return note_time_to_pixel_index(visible_led_count + index, Note::Direction::Clockwise);
        }
        return static_cast<std::int64_t>(current_time_ms) + static_cast<int64_t>(index) * static_cast<int64_t>(header.ms_per_pixel);
    }
}
