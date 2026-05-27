#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "ohos_init.h"
#include "ohos_types.h"
#include "smart_shed_shared.h"
#include "wifiiot_errno.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"

#define AHT20_I2C_SDA_PIN WIFI_IOT_IO_NAME_GPIO_13
#define AHT20_I2C_SCL_PIN WIFI_IOT_IO_NAME_GPIO_14
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0
#define AHT20_BAUDRATE (400 * 1000)

#define AHT20_DEVICE_ADDR 0x38
#define AHT20_READ_ADDR ((AHT20_DEVICE_ADDR << 1) | 0x1)
#define AHT20_WRITE_ADDR ((AHT20_DEVICE_ADDR << 1) | 0x0)

#define AHT20_CMD_CALIBRATION 0xBE
#define AHT20_CMD_CALIBRATION_ARG0 0x08
#define AHT20_CMD_CALIBRATION_ARG1 0x00
#define AHT20_CMD_TRIGGER 0xAC
#define AHT20_CMD_TRIGGER_ARG0 0x33
#define AHT20_CMD_TRIGGER_ARG1 0x00
#define AHT20_CMD_RESET 0xBA
#define AHT20_CMD_STATUS 0x71

#define AHT20_STARTUP_TIME_US (20 * 1000)
#define AHT20_CALIBRATION_TIME_US (40 * 1000)
#define AHT20_MEASURE_TIME_US (75 * 1000)
#define AHT20_STATUS_RESPONSE_MAX 6
#define AHT20_RESOLUTION (1 << 20)
#define AHT20_MAX_RETRY 10

#define AHT20_STATUS_BUSY_SHIFT 7
#define AHT20_STATUS_BUSY_MASK (0x1 << AHT20_STATUS_BUSY_SHIFT)
#define AHT20_STATUS_BUSY(status) (((status) & AHT20_STATUS_BUSY_MASK) >> AHT20_STATUS_BUSY_SHIFT)

#define AHT20_STATUS_CALI_SHIFT 3
#define AHT20_STATUS_CALI_MASK (0x1 << AHT20_STATUS_CALI_SHIFT)
#define AHT20_STATUS_CALI(status) (((status) & AHT20_STATUS_CALI_MASK) >> AHT20_STATUS_CALI_SHIFT)

int temp = 0;
int hum = 0;

static uint32_t AHT20_Read(uint8_t *buffer, uint32_t buffLen)
{
    WifiIotI2cData data = {0};
    data.receiveBuf = buffer;
    data.receiveLen = buffLen;

    smart_shed_i2c0_lock();
    uint32_t retval = I2cRead(AHT20_I2C_IDX, AHT20_READ_ADDR, &data);
    smart_shed_i2c0_unlock();
    if (retval != WIFI_IOT_SUCCESS) {
        printf("I2cRead() failed, %0X!\n", retval);
        return retval;
    }
    return WIFI_IOT_SUCCESS;
}

static uint32_t AHT20_Write(uint8_t *buffer, uint32_t buffLen)
{
    WifiIotI2cData data = {0};
    data.sendBuf = buffer;
    data.sendLen = buffLen;

    smart_shed_i2c0_lock();
    uint32_t retval = I2cWrite(AHT20_I2C_IDX, AHT20_WRITE_ADDR, &data);
    smart_shed_i2c0_unlock();
    if (retval != WIFI_IOT_SUCCESS) {
        printf("I2cWrite(%02X) failed, %0X!\n", buffer[0], retval);
        return retval;
    }
    return WIFI_IOT_SUCCESS;
}

static uint32_t AHT20_StatusCommand(void)
{
    uint8_t statusCmd[] = {AHT20_CMD_STATUS};
    return AHT20_Write(statusCmd, sizeof(statusCmd));
}

static uint32_t AHT20_ResetCommand(void)
{
    uint8_t resetCmd[] = {AHT20_CMD_RESET};
    return AHT20_Write(resetCmd, sizeof(resetCmd));
}

static uint32_t AHT20_CalibrateCommand(void)
{
    uint8_t calibrateCmd[] = {
        AHT20_CMD_CALIBRATION,
        AHT20_CMD_CALIBRATION_ARG0,
        AHT20_CMD_CALIBRATION_ARG1,
    };
    return AHT20_Write(calibrateCmd, sizeof(calibrateCmd));
}

static uint32_t AHT20_Calibrate(void)
{
    uint8_t buffer[AHT20_STATUS_RESPONSE_MAX] = {0};

    uint32_t retval = AHT20_StatusCommand();
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    retval = AHT20_Read(buffer, sizeof(buffer));
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    if (AHT20_STATUS_BUSY(buffer[0]) || !AHT20_STATUS_CALI(buffer[0])) {
        retval = AHT20_ResetCommand();
        if (retval != WIFI_IOT_SUCCESS) {
            return retval;
        }
        usleep(AHT20_STARTUP_TIME_US);

        retval = AHT20_CalibrateCommand();
        usleep(AHT20_CALIBRATION_TIME_US);
        return retval;
    }

    return WIFI_IOT_SUCCESS;
}

static uint32_t AHT20_StartMeasure(void)
{
    uint8_t triggerCmd[] = {
        AHT20_CMD_TRIGGER,
        AHT20_CMD_TRIGGER_ARG0,
        AHT20_CMD_TRIGGER_ARG1,
    };
    return AHT20_Write(triggerCmd, sizeof(triggerCmd));
}

static uint32_t AHT20_GetMeasureResult(float *temperature, float *humidity)
{
    if (temperature == NULL || humidity == NULL) {
        return WIFI_IOT_FAILURE;
    }

    uint8_t buffer[AHT20_STATUS_RESPONSE_MAX] = {0};
    uint32_t retval = AHT20_Read(buffer, sizeof(buffer));
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    for (uint32_t i = 0; AHT20_STATUS_BUSY(buffer[0]) && i < AHT20_MAX_RETRY; ++i) {
        usleep(AHT20_MEASURE_TIME_US);
        retval = AHT20_Read(buffer, sizeof(buffer));
        if (retval != WIFI_IOT_SUCCESS) {
            return retval;
        }
    }

    if (AHT20_STATUS_BUSY(buffer[0])) {
        printf("AHT20 device always busy!\r\n");
        return WIFI_IOT_FAILURE;
    }

    uint32_t humiRaw = buffer[1];
    humiRaw = (humiRaw << 8) | buffer[2];
    humiRaw = (humiRaw << 4) | ((buffer[3] & 0xF0) >> 4);
    *humidity = humiRaw / (float)AHT20_RESOLUTION * 100.0f;

    uint32_t tempRaw = buffer[3] & 0x0F;
    tempRaw = (tempRaw << 8) | buffer[4];
    tempRaw = (tempRaw << 8) | buffer[5];
    *temperature = tempRaw / (float)AHT20_RESOLUTION * 200.0f - 50.0f;

    return WIFI_IOT_SUCCESS;
}

static void AHT20_Init(void)
{
    IoSetFunc(AHT20_I2C_SDA_PIN, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(AHT20_I2C_SCL_PIN, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    smart_shed_i2c0_init();
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);
}

void temp_and_hum_thread(void *arg)
{
    (void)arg;
    float temperature = 0.0f;
    float humidity = 0.0f;

    AHT20_Init();
    while (AHT20_Calibrate() != WIFI_IOT_SUCCESS) {
        printf("AHT20 sensor init failed!\r\n");
        usleep(1000);
    }

    while (1) {
        if (AHT20_StartMeasure() == WIFI_IOT_SUCCESS &&
            AHT20_GetMeasureResult(&temperature, &humidity) == WIFI_IOT_SUCCESS) {
            temp = (int)temperature;
            hum = (int)humidity;
        }
        sleep(1);
    }
}

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
    if (osThreadNew(temp_and_hum_thread, NULL, &attr) == NULL) {
        printf("[temp_and_hum_thread] Falied to create temp_and_hum_thread!\n");
    }
}
