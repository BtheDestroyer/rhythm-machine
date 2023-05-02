#include "lcd.h"
#include <stdio.h>

static bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scan_i2c_bus()
{
    i2c_init(LCD_I2C_CHANNEL, 100 * 1000);
    gpio_set_function(LCD_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(LCD_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(LCD_SDA_PIN);
    gpio_pull_up(LCD_SCL_PIN);
    printf("I2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}

I2C_LCD::I2C_LCD()
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

void I2C_LCD::send_character(char character)
{
    send_byte(static_cast<std::uint8_t>(character), SendMode::Character);
}

void I2C_LCD::send_command(Command command)
{
    send_byte(
        static_cast<std::uint8_t>(command),
        SendMode::Command
    );
}

void I2C_LCD::clear()
{
    send_command(Command::ClearDisplay);
    move_cursor(0, 0);
}

void I2C_LCD::display(std::string_view str, bool clear_first /* = true */)
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

void I2C_LCD::move_cursor(std::uint8_t line, std::uint8_t position)
{
    send_command(static_cast<Command>((line == 0 ? 0x80 : 0xC0) + position));
}

void I2C_LCD::toggle_enable(std::uint8_t val)
{
    constexpr static std::uint32_t delay_us{ 600u };
    constexpr static std::uint8_t lcd_enable_bit{ 1 << 2 };
    sleep_us(delay_us);
    i2c_write_byte(val | lcd_enable_bit);
    sleep_us(delay_us);
    i2c_write_byte(val & ~lcd_enable_bit);
    sleep_us(delay_us);
}

void I2C_LCD::send_byte(std::uint8_t byte, SendMode mode)
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
