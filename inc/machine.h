#pragma once
#include <memory>
#include "audio.h"
#include "input.h"
#include "lcd.h"
#include "leds.h"
#include "sd.h"
#include "state.h"
#include "song_data.h"

namespace States
{
    class SongList : public State
    {
    private:
        std::vector<std::string> songs;
        std::uint32_t last_song_index{~0u};
        std::size_t current_index{0};
        
    public:
        SongList(Machine &machine);
        void operator()(Machine &machine) override;
    };

    class PlaySong : public State
    {
    private:
        std::uint32_t score{ 0u };
        void increment_score(Machine& machine, std::uint32_t val);
        song_data::Song song;
        std::uint64_t last_update_ms{ ~0ull };

    public:
        PlaySong(Machine& machine);
        void operator()(Machine& machine) override;
    };
}

struct Machine
{
    Machine();

    void update();

    template <typename TState>
    void switch_state()
    {
        next_state = std::make_unique<TState>(*this);
    }

    I2C_LCD lcd;
    LEDs leds;
    SDCard sd;
    struct Buttons
    {
        struct ColorPair
        {
            Button red;
            Button green;
            Button blue;
        } left, right;
    } buttons{{17, 18, 19}, {20, 21, 22}};

    [[nodiscard]] inline std::uint32_t get_current_tick() { return current_tick; }
    
    std::string current_song_path;

private:
    std::uint32_t current_tick{0};

    std::unique_ptr<State> current_state;
    std::unique_ptr<State> next_state;
};
