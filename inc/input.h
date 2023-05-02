#pragma once
#include <cstdint>

class Button
{
public:
    Button(std::uint32_t gpio_pin);

    enum class State : std::uint8_t
    {
        Uninitialized,
        Open,
        Pressed,
        Held,
        Released,
    };

    void update();

    [[nodiscard]] inline const State& get_state() const
    {
        return last_state;
    }

private:
    std::uint32_t gpio_pin;
    State last_state{ State::Uninitialized };
};
