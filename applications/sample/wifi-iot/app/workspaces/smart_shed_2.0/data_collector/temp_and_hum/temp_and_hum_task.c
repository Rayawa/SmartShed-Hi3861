#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "ohos_types.h"

#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "wifiiot_errno.h"
#include "smart_shed_shared.h"

#define SHT40_I2C_IDX 0                 // I2C设备号
#define SHT40_I2C_BAUDRATE (400 * 1000) // I2C波特率
#define SHT40_ADDR 0x44                 // SHT40设备地址
#define SHT40_STATUS_RESPONSE_MAX 6     //读取传感器数据长度

#define SHT40_CMD_TRIGGER 0xFD //高精度测量命令

int temp = 0;
int hum = 0;

/*
 * 函数名称 : SHT40_Init
 * 功能描述 : 温湿度传感器硬件初始化
 * 参    数 : 无
 * 返回值   : 无
 * 示    例 : SHT40_Init();
 */
/******************************************************************************/
void SHT40_Init(void)
/******************************************************************************/
{
    smart_shed_i2c0_init();
    I2cInit(SHT40_I2C_IDX, SHT40_I2C_BAUDRATE); // I2C初始化
}


/*
* 函数名称 : SHT40_Write
* 功能描述 : 写命令
* 参    数 : buffer - 写入数据指针
             buffLen - 写入数据长度
* 返回值   : 0 - 写入成功
            -1 - 写入失败
* 示    例 : result = SHT40_Write(&buffer,buffLen);
*/
/******************************************************************************/
static uint32_t SHT40_Write(uint8_t *buffer, uint32_t buffLen)
/******************************************************************************/
{
    WifiIotI2cData i2cData = {buffer, buffLen, NULL, 0};
    smart_shed_i2c0_lock();
    uint32_t retval = I2cWrite(SHT40_I2C_IDX, (SHT40_ADDR << 1) | 0, &i2cData);
    smart_shed_i2c0_unlock();
    if (retval != 0)
    {
        printf("I2cWrite(%02X) failed, %0X!\n", buffer[0], retval);
        return retval;
    }
    return 0;
}

/*
* 函数名称 : SHT40_Read
* 功能描述 : 读取温湿度数据
* 参    数 : buffer - 读取数据指针
             buffLen - 读取数据长度
* 返回值   : 0 - 读取成功
            -1 - 读取失败
* 示    例 : result = SHT40_Read(&buffer,buffLen);
*/
static uint32_t SHT40_Read(uint8_t *buffer, uint32_t buffLen)
/******************************************************************************/
{
    uint32_t retval;
    WifiIotI2cData i2cData = {NULL, 0, buffer, buffLen};
    smart_shed_i2c0_lock();
    retval = I2cRead(SHT40_I2C_IDX, (SHT40_ADDR << 1) | 1, &i2cData);
    smart_shed_i2c0_unlock();
    if (retval != 0)
    {
        printf("I2cRead() failed, %0X!\n", retval);
        return retval;
    }
    return 0;
}

/*
* 函数名称 : SHT40_StartMeasure
* 功能描述 : 开始测量
* 参    数 : temp - 温度值
             humi - 湿度值
* 返回值   : 0 - 操作成功
            -1 - 操作失败
* 示    例 : result = SHT40_StartMeasure();
*/
/******************************************************************************/
uint32_t SHT40_StartMeasure(void)
/******************************************************************************/
{
    uint8_t triggerCmd[] = {SHT40_CMD_TRIGGER};
    return SHT40_Write(triggerCmd, sizeof(triggerCmd));
}

/*
* 函数名称 : SHT40_GetMeasureResult
* 功能描述 : 获取测量结果，拼接转换为标准值
* 参    数 : temp - 温度值
             humi - 湿度值
* 返回值   : 0 - 测量成功
            -1 - 测量失败
* 示    例 : result = SHT40_GetMeasureResult(&temp,humi);
*/
/******************************************************************************/
uint32_t SHT40_GetMeasureResult(int *temp, int *humi)
/******************************************************************************/
{

    uint32_t retval = 0;

    float t_degC = 0;
    float rh_pRH = 0;
    float t_ticks = 0.0;
    float rh_ticks = 0.0;

    if (temp == NULL || humi == NULL)
    {
        return -1;
    }

    uint8_t buffer[SHT40_STATUS_RESPONSE_MAX] = {0};
    memset(buffer, 0x0, sizeof(buffer));
    retval = SHT40_Read(buffer, sizeof(buffer)); // recv status command result
    if (retval != 0)
    {
        return retval;
    }

    t_ticks = buffer[0] * 256 + buffer[1];
    rh_ticks = buffer[3] * 256 + buffer[4];
    t_degC = -45 + 175 * t_ticks / 65535;
    rh_pRH = -6 + 125 * rh_ticks / 65535;
    if (rh_pRH >= 100)
    {
        rh_pRH = 100;
    }
    if (rh_pRH < 0)
    {
        rh_pRH = 0;
    }
    *humi = (uint8_t)rh_pRH;

    *temp = (uint8_t)t_degC;

    return 0;
}


//业务函数，完成数据在OLED上显示
void temp_and_hum_thread(void *arg)
{
    (void)arg;
    SHT40_Init(); //温湿度传感器IO初始化

    while (1)
    {
        SHT40_StartMeasure();
        usleep(20 * 1000);
        SHT40_GetMeasureResult(&temp, &hum); //获取当前温湿度值
        sleep(1);                            //睡眠
    }
}

//创建新线程运行OledTask函数
void temp_and_hum_task(void)
{
    osThreadAttr_t attr;
    attr.name = "temp_and_hum_thread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(temp_and_hum_thread, NULL, &attr) == NULL)
    {
        printf("[temp_and_hum_thread] Falied to create temp_and_hum_thread!\n");
    }
}
