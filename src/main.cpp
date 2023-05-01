#include <sstream>
#include "pico/stdlib.h"   // stdlib
#include "machine.h"

int main()
{
    stdio_init_all();

    printf("Hello\n");
    Machine machine;

    while (1)
    {
        machine.update();
    }
}
