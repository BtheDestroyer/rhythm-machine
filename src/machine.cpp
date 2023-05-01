#include "machine.h"

namespace States
{
    SongList::SongList(Machine &machine)
    {
        machine.lcd.display("Loading songs...");
        for (const SDCard::FileEntry &entry : machine.sd.get_file_list("/"))
        {
            if (entry.type != SDCard::FileEntry::FileType::Directory)
            {
                continue;
            }
            std::optional<FILINFO> file_info{
                machine.sd.get_file_info((entry.name + "/song.wav").c_str())};
            if (!file_info.has_value())
            {
                continue;
            }
            songs.emplace_back(entry.name);
        }
    }

    void SongList::operator()(Machine &machine)
    {
        if (current_index != last_song_index)
        {
            if (songs.size() > 0)
            {
                machine.lcd.display(songs[current_index]);
            }
            else
            {
                machine.lcd.display("No songs on SD");
            }
            last_song_index = current_index;
        }
        if (songs.size() == 0)
        {
            return;
        }
        if (machine.buttons.right.blue.get_state() == Button::State::Pressed)
        {
            Audio::start_streaming_wave({(songs[current_index] + "/song.wav").c_str()});
            machine.switch_state<PlaySong>();
            return;
        }
        if (machine.buttons.right.red.get_state() == Button::State::Pressed)
        {
            current_index = (current_index + 1) % songs.size();
        }
        if (machine.buttons.left.red.get_state() == Button::State::Pressed)
        {
            if (current_index == 0)
            {
                current_index = songs.size() - 1;
            }
            else
            {
                --current_index;
            }
        }

        // Attract mode
        machine.leds.pattern_snakes(machine.get_current_tick());
        sleep_ms(12);
    }

    PlaySong::PlaySong(Machine &machine)
    {
        machine.leds.clear();
        machine.lcd.display(std::to_string(score));
    }

    void PlaySong::operator()(Machine &machine)
    {
        // machine.leds.display_notes(active_notes);
    }

    void PlaySong::increment_score(Machine &machine, std::uint32_t val)
    {
        score += val;
        machine.lcd.display(std::to_string(score));
    }

}