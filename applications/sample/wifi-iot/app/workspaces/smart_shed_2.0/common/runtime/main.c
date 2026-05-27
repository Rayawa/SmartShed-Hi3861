#include <stdio.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "ohos_init.h"

#ifdef SMART_SHED_ENABLE_FAN
#include "fan_task.h"
#endif
#ifdef SMART_SHED_ENABLE_LED
#include "led_task.h"
#endif
#ifdef SMART_SHED_ENABLE_LIGHT_INTENSITY
#include "light_intensity_task.h"
#endif
#include "smart_shed_mqtt.h"
#ifdef SMART_SHED_ENABLE_OLED
#include "smart_shed_oled.h"
#endif
#ifdef SMART_SHED_ENABLE_SOIL_MOISTURE
#include "soil_moisture_task.h"
#endif
#ifdef SMART_SHED_ENABLE_TEMP_AND_HUM
#include "temp_and_hum_task.h"
#endif
#ifdef SMART_SHED_ENABLE_WATER_PUMP
#include "water_pump_task.h"
#endif

static void smart_shed_thread(void *arg)
{
    (void)arg;
    sleep(1);
    printf("Smart Shed combined app running\n");

#ifdef SMART_SHED_ENABLE_SOIL_MOISTURE
    soil_moisture_task();
#endif
#ifdef SMART_SHED_ENABLE_LIGHT_INTENSITY
    light_intensity_task();
#endif
#ifdef SMART_SHED_ENABLE_TEMP_AND_HUM
    temp_and_hum_task();
#endif
#ifdef SMART_SHED_ENABLE_FAN
    fan_task();
#endif
#ifdef SMART_SHED_ENABLE_LED
    led_task();
#endif
#ifdef SMART_SHED_ENABLE_WATER_PUMP
    water_pump_task();
#endif
#ifdef SMART_SHED_ENABLE_OLED
    smart_shed_oled_task();
#endif
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
