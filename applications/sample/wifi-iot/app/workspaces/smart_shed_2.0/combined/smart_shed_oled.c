#include <stdio.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "light_intensity_task.h"
#include "oled_ssd1306.h"
#include "smart_shed_oled.h"
#include "smart_shed_shared.h"
#include "soil_moisture_task.h"
#include "temp_and_hum_task.h"

static void smart_shed_oled_thread(void *arg)
{
    (void)arg;
    OledInit();

    while (1) {
        char line[32];

        OledFillScreen(0x00);
        OledShowString(0, 0, "smart shed 2.0", FONT6x8);

        snprintf(line, sizeof(line), "soil:%3d", moisture);
        OledShowString(0, 2, line, FONT6x8);

        snprintf(line, sizeof(line), "light:%3d", intensity);
        OledShowString(0, 3, line, FONT6x8);

        snprintf(line, sizeof(line), "temp:%3d", temp);
        OledShowString(0, 4, line, FONT6x8);

        snprintf(line, sizeof(line), "hum :%3d", hum);
        OledShowString(0, 5, line, FONT6x8);

        snprintf(line, sizeof(line), "f%d p%d l%03d", fan_level, water_pump_level, led_level);
        OledShowString(0, 6, line, FONT6x8);
        sleep(1);
    }
}

void smart_shed_oled_task(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "smart_shed_oled";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(smart_shed_oled_thread, NULL, &attr) == NULL) {
        printf("[smart_shed_oled] Failed to create oled thread!\n");
    }
}
