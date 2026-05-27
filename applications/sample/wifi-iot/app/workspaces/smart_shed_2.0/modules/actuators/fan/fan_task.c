#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "smart_shed_shared.h"

#define FAN_CTRL_PIN WIFI_IOT_IO_NAME_GPIO_8

void fan_task_init(void)
{
    GpioInit();
    IoSetFunc(FAN_CTRL_PIN, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(FAN_CTRL_PIN, WIFI_IOT_GPIO_DIR_OUT);
}

void fan_thread(void *arg)
{
    (void)arg;
    fan_task_init(); //初始化

    while (1)
    {
        if (fan_level > 0) {
            GpioSetOutputVal(FAN_CTRL_PIN, WIFI_IOT_GPIO_VALUE1);
        } else {
            GpioSetOutputVal(FAN_CTRL_PIN, WIFI_IOT_GPIO_VALUE0);
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
