#include <stdio.h>
#include <pico/stdlib.h>
#include "config.h"

int main(void)
{
    int toggle = 0;
    stdio_init_all();
    gpio_init(OUT_PIN);
    gpio_set_dir(OUT_PIN, GPIO_OUT);
    gpio_put(OUT_PIN, toggle);

    while (true) {
        toggle = !toggle;
        gpio_put(OUT_PIN, toggle);
        for (int i=0; i < 2000000; i++);
        sleep_ms(DELAY_MS);
    }
}
