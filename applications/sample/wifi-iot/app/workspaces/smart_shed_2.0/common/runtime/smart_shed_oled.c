#include <stdio.h>
#include <unistd.h>

#include "cmsis_os2.h"

#ifdef SMART_SHED_ENABLE_LIGHT_INTENSITY
#include "light_intensity_task.h"
#endif
#include "oled_ssd1306.h"
#include "smart_shed_oled.h"
#include "smart_shed_shared.h"
#ifdef SMART_SHED_ENABLE_SOIL_MOISTURE
#include "soil_moisture_task.h"
#endif
#ifdef SMART_SHED_ENABLE_TEMP_AND_HUM
#include "temp_and_hum_task.h"
#endif

static void smart_shed_oled_thread(void *arg)
{
    (void)arg;
    OledInit();

    while (1) {
        char line[32];
        int row = 2;

        OledFillScreen(0x00);
        OledShowString(0, 0, "smart shed 2.0", FONT6x8);

#ifdef SMART_SHED_ENABLE_SOIL_MOISTURE
        snprintf(line, sizeof(line), "soil:%3d", moisture);
        OledShowString(0, row++, line, FONT6x8);
#endif

#ifdef SMART_SHED_ENABLE_LIGHT_INTENSITY
        snprintf(line, sizeof(line), "light:%3d", intensity);
        OledShowString(0, row++, line, FONT6x8);
#endif

#ifdef SMART_SHED_ENABLE_TEMP_AND_HUM
        snprintf(line, sizeof(line), "temp:%3d", temp);
        OledShowString(0, row++, line, FONT6x8);

        snprintf(line, sizeof(line), "hum :%3d", hum);
        OledShowString(0, row++, line, FONT6x8);
#endif

#ifdef SMART_SHED_ENABLE_LED
        snprintf(line, sizeof(line), "led :%3d", led_level);
        OledShowString(0, row++, line, FONT6x8);
#endif
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
