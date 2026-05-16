#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "mqtt_task.h"

#define FAN_PIN_NAME WIFI_IOT_IO_NAME_GPIO_8
#define FAN_POWER_PIN WIFI_IOT_IO_NAME_GPIO_12

#define PWM_FREQ_DIVITION 64000

void fan_task_init(void)
{
    GpioInit();
    IoSetFunc(FAN_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_8_PWM1_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM1); //

    // IoSetFunc(FAN_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    IoSetFunc(FAN_POWER_PIN, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    // GpioSetDir(FAN_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(FAN_POWER_PIN, WIFI_IOT_GPIO_DIR_OUT);
}

void fan_thread(void *arg)
{
    (void)arg;
    fan_task_init(); //初始化

    while (1)
    {
        // printf("fan_level:%d\r\n", fan_level);
        // if (fan_level)
        //     GpioSetOutputVal(FAN_PIN_NAME, WIFI_IOT_GPIO_VALUE1);
        // else
        //     GpioSetOutputVal(FAN_PIN_NAME, WIFI_IOT_GPIO_VALUE0);
        if (fan_level > 0 && fan_level < 4)
        {
            GpioSetOutputVal(FAN_POWER_PIN, WIFI_IOT_GPIO_VALUE1);
            PwmStart(WIFI_IOT_PWM_PORT_PWM1, PWM_FREQ_DIVITION / 10 * (fan_level + 7), PWM_FREQ_DIVITION);
        }
        else
        {
            GpioSetOutputVal(FAN_POWER_PIN, WIFI_IOT_GPIO_VALUE0);
            PwmStop(WIFI_IOT_PWM_PORT_PWM1);
        }
        sleep(1); //睡眠
    }
}

//创建新线程运行OledTask函数
void fan_task(void)
{
    osThreadAttr_t attr;
    attr.name = "fan_thread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(fan_thread, NULL, &attr) == NULL)
    {
        printf("[led_thread] Falied to create fan_thread!\n");
    }
}
