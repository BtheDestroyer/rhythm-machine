#pragma once
#include <cstdint>
#include "pico/stdlib.h"   // stdlib

class Button
{
public:
    Button(std::uint32_t gpio_pin)
        : gpio_pin{ gpio_pin }
    {
        gpio_set_dir(gpio_pin, GPIO_IN);
        gpio_set_pulls(gpio_pin, true, false);
    }

    enum class State : std::uint8_t
    {
        Uninitialized,
        Open,
        Pressed,
        Held,
        Released,
    };

    void update()
    {
        const bool is_pressed{ !gpio_get(gpio_pin) };

        switch (last_state)
        {
        case State::Uninitialized:
            if (is_pressed)
            {
                last_state = State::Pressed;
            }
            else
            {
                last_state = State::Open;
            }
            break;
        case State::Released:
            last_state = State::Open;
            [[fallthrough]];
        case State::Open:
            if (is_pressed)
            {
                last_state = State::Pressed;
            }
            break;
        case State::Pressed:
            last_state = State::Held;
            [[fallthrough]];
        case State::Held:
            if (!is_pressed)
            {
                last_state = State::Released;
            }
            break;
        }
    }

    const State& get_state() const
    {
        return last_state;
    }

private:
    std::uint32_t gpio_pin;
    State last_state{ State::Uninitialized };
};
