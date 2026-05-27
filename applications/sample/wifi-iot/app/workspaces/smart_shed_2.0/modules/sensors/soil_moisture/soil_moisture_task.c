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

int moisture = 0;

static void soil_moisture_thread(void *arg)
{
    (void)arg;
    unsigned short data = 0; //保存读取到土壤湿度值
    GpioInit();
    while (1)
    {

        //调用AdcRead读取值
        if (AdcRead(SOIL_MOISTURE_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            moisture = (2200 - data) / 10;
        }

        sleep(1);
    }
}

void soil_moisture_task(void)
{
    osThreadAttr_t attr;
    attr.name = "soil_moisture_thread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(soil_moisture_thread, NULL, &attr) == NULL)
    {
        printf("[soil_moisture_thread] Falied to create soil_moisture_thread!\n");
    }
}
