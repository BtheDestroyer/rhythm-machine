#pragma once
#include <memory>
#include "audio.h"
#include "input.h"
#include "lcd.h"
#include "leds.h"
#include "sd.h"
#include "state.h"

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
        std::uint32_t score{ 0 };
        void increment_score(Machine& machine, std::uint32_t val);

    public:
        PlaySong(Machine& machine);
        void operator()(Machine& machine) override;
    };
}

struct Machine
{
    Machine()
    {
        Audio::init();
        lcd.send_command(I2C_LCD::Command::DisplayControl, I2C_LCD::DisplayFlag::DisplayOn);
        lcd.display("Hello, LCD");
        lcd.display("Reading SD...");
        if (!sd.init())
        {
            lcd.display("SD Card Error!");
            exit(1);
        }
        current_state = std::make_unique<States::SongList>(*this);
    }

    void update()
    {
        buttons.left.red.update();
        buttons.left.green.update();
        buttons.left.blue.update();
        buttons.right.red.update();
        buttons.right.green.update();
        buttons.right.blue.update();
        Audio::stream_wave_to_inactive_buffer();

        if (current_state)
        {
            (*current_state)(*this);
            ++current_tick;
            if (next_state)
            {
                current_state = std::move(next_state);
            }
        }
    }

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

    std::uint32_t get_current_tick() { return current_tick; }
    
private:
    std::uint32_t current_tick{0};

    std::unique_ptr<State> current_state;
    std::unique_ptr<State> next_state;
};
