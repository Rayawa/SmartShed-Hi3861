#include <stdio.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "ohos_init.h"

#include "fan_task.h"
#include "led_task.h"
#include "light_intensity_task.h"
#include "smart_shed_mqtt.h"
#include "smart_shed_oled.h"
#include "soil_moisture_task.h"
#include "temp_and_hum_task.h"
#include "water_pump_task.h"

static void smart_shed_thread(void *arg)
{
    (void)arg;
    sleep(1);
    printf("Smart Shed combined app running\n");

    soil_moisture_task();
    light_intensity_task();
    temp_and_hum_task();

    fan_task();
    led_task();
    water_pump_task();

    smart_shed_oled_task();
    smart_shed_mqtt_task();
}

void ss_entry(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "smart_shed_all";
    attr.stack_size = 8192;
    attr.priority = osPriorityNormal;

    if (osThreadNew(smart_shed_thread, NULL, &attr) == NULL) {
        printf("[smart_shed_all] Failed to create main thread!\n");
    }
}

SYS_RUN(ss_entry);
