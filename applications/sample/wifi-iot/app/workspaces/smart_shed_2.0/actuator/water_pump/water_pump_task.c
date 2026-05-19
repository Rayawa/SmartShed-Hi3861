#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "smart_shed_shared.h"

#define WATER_PUMP_PIN_NAME WIFI_IOT_IO_NAME_GPIO_10
#define WATER_PUMP_POWER_PIN WIFI_IOT_IO_NAME_GPIO_5

#define PWM_FREQ_DIVITION 64000

void water_pump_task_init(void)
{
    GpioInit();
    IoSetFunc(WATER_PUMP_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM1); //

    IoSetFunc(WATER_PUMP_POWER_PIN, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(WATER_PUMP_POWER_PIN, WIFI_IOT_GPIO_DIR_OUT);
}

void water_pump_thread(void *arg)
{
    (void)arg;
    water_pump_task_init(); //初始化

    while (1)
    {
        // printf("water_pump_level:%d\r\n", water_pump_level);
        if (water_pump_level > 0 && water_pump_level < 4)
        {
            GpioSetOutputVal(WATER_PUMP_POWER_PIN, WIFI_IOT_GPIO_VALUE1);
            PwmStart(WIFI_IOT_PWM_PORT_PWM1, PWM_FREQ_DIVITION / 10 * (water_pump_level + 7), PWM_FREQ_DIVITION);
        }
        else
        {
            GpioSetOutputVal(WATER_PUMP_POWER_PIN, WIFI_IOT_GPIO_VALUE0);
            PwmStop(WIFI_IOT_PWM_PORT_PWM1);
        }
        sleep(1); //睡眠
    }
}

void water_pump_task(void)
{
    osThreadAttr_t attr;
    attr.name = "water_pump_thread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(water_pump_thread, NULL, &attr) == NULL)
    {
        printf("[water_pump_thread] Falied to create water_pump_thread!\n");
    }
}
