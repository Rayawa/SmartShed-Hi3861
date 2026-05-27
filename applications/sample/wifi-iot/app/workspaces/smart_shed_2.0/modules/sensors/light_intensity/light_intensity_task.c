#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "ohos_types.h"

#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#define SOIL_MOISTURE_CHAN_NAME WIFI_IOT_ADC_CHANNEL_4

int intensity = 0;

static void light_intensity_thread(void *arg)
{
    (void)arg;
    unsigned short data = 0; //保存读取到土壤湿度值
    GpioInit();
    while (1)
    {

        //调用AdcRead读取值
        if (AdcRead(SOIL_MOISTURE_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            intensity = 2000 - data;
        }

        sleep(1);
    }
}

//创建新线程运行OledTask函数
void light_intensity_task(void)
{
    osThreadAttr_t attr;
    attr.name = "light_intensity_thread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(light_intensity_thread, NULL, &attr) == NULL)
    {
        printf("[light_intensity_thread] Falied to create light_intensity_thread!\n");
    }
}
