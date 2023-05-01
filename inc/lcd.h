#pragma once
#include <cstdint>
#include <string_view>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define LCD_SDA_PIN 8
#define LCD_SCL_PIN LCD_SDA_PIN + 1
static_assert((LCD_SDA_PIN >= 0 && LCD_SDA_PIN < 20) || LCD_SDA_PIN == 26, "LCD_SDA_PIN must be between 0 and 20 or 26");
static_assert(LCD_SDA_PIN % 2 == 0, "LCD_SDA_PIN must be a multiple of 2");
#if (LCD_SDA_PIN % 4) >> 1 == 0
#define LCD_I2C_CHANNEL i2c0
#else
#define LCD_I2C_CHANNEL i2c1
#endif

#define LCD_I2C_ADDR 0x27

void scan_i2c_bus();

inline void i2c_write_byte(std::uint8_t byte)
{
    i2c_write_blocking(LCD_I2C_CHANNEL, LCD_I2C_ADDR, &byte, 1, false);
}

class I2C_LCD
{
public:
    enum class Command : std::uint8_t {
        ClearDisplay = 1 << 0,
        ReturnHome = 1 << 1,
        EntryModeSet = 1 << 2,
        DisplayControl = 1 << 3,
        CursorShift = 1 << 4,
        FunctionSet = 1 << 5,
        SetCGRAMAddr = 1 << 6,
        SetDDRAMAddr = 1 << 7,
    };

    enum class EntryModeFlag : std::uint8_t {
        EntryShiftIncrement = 1 << 0,
        EntryLeft = 1 << 1,
    };

    enum class DisplayFlag : std::uint8_t {
        BlinkOn = 1 << 0,
        CursorOn = 1 << 1,
        DisplayOn = 1 << 2,
    };

    enum class FunctionSetFlag : std::uint8_t {
        FiveByTenDots = 1 << 2,
        TwoLine = 1 << 3,
        EightBitMode = 1 << 4,
    };

    enum class BacklightFlag : std::uint8_t {
        BacklightOn = 1 << 3,
    };

    I2C_LCD()
    {
        i2c_init(LCD_I2C_CHANNEL, 100'000);
        gpio_set_function(LCD_SDA_PIN, GPIO_FUNC_I2C);
        gpio_set_function(LCD_SCL_PIN, GPIO_FUNC_I2C);
        gpio_pull_up(LCD_SDA_PIN);
        gpio_pull_up(LCD_SCL_PIN);

        send_command(static_cast<Command>(0x03));
        send_command(static_cast<Command>(0x03));
        send_command(static_cast<Command>(0x03));
        send_command(Command::ReturnHome);

        send_command(Command::EntryModeSet, EntryModeFlag::EntryLeft);
        send_command(Command::FunctionSet, FunctionSetFlag::TwoLine);
        send_command(Command::DisplayControl, DisplayFlag::DisplayOn);
        display("0123456789");
    }

    void send_character(char character)
    {
        send_byte(static_cast<std::uint8_t>(character), SendMode::Character);
    }

    template <typename ...TCommandFlags>
    void send_command(Command command, TCommandFlags... flags)
    {
        send_byte(
            (static_cast<std::uint8_t>(command) | ... | static_cast<std::uint8_t>(flags)),
            SendMode::Command
        );
    }

    void send_command(Command command)
    {
        send_byte(
            static_cast<std::uint8_t>(command),
            SendMode::Command
        );
    }

    void clear()
    {
        send_command(Command::ClearDisplay);
        move_cursor(0, 0);
    }

    void display(std::string_view str, bool clear_first = true)
    {
        if (clear_first)
        {
            clear();
        }
        for (char character : str)
        {
            send_character(character);
        }
    }

    void move_cursor(std::uint8_t line, std::uint8_t position)
    {
        send_command(static_cast<Command>((line == 0 ? 0x80 : 0xC0) + position));
    }

private:
    enum class SendMode : std::uint8_t {
        Command = 0,
        Character = 1,
    };

    void toggle_enable(std::uint8_t val)
    {
        constexpr static std::uint32_t delay_us{ 600u };
        constexpr static std::uint8_t lcd_enable_bit{ 1 << 2 };
        sleep_us(delay_us);
        i2c_write_byte(val | lcd_enable_bit);
        sleep_us(delay_us);
        i2c_write_byte(val & ~lcd_enable_bit);
        sleep_us(delay_us);
    }
 
    void send_byte(std::uint8_t byte, SendMode mode)
    {
        const std::uint8_t high{
            static_cast<std::uint8_t>(
                static_cast<std::uint8_t>(mode)
                | (byte & 0xF0)
                | static_cast<std::uint8_t>(BacklightFlag::BacklightOn)
            )
        };
        const std::uint8_t low{
            static_cast<std::uint8_t>(
                static_cast<std::uint8_t>(mode)
                | ((byte << 4) & 0xF0)
                | static_cast<std::uint8_t>(BacklightFlag::BacklightOn)
            )
        };

        i2c_write_byte(high);
        toggle_enable(high);
        i2c_write_byte(low);
        toggle_enable(low);
    }
};