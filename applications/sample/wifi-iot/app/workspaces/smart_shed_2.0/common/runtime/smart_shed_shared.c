#include "smart_shed_shared.h"

#include "cmsis_os2.h"

int fan_level = 0;
int led_level = 0;
int water_pump_level = 0;

static osMutexId_t g_i2c0_mutex = NULL;

void smart_shed_i2c0_init(void)
{
    static osMutexAttr_t attr = {
        .name = "smart_shed_i2c0",
    };

    if (g_i2c0_mutex == NULL) {
        g_i2c0_mutex = osMutexNew(&attr);
    }
}

void smart_shed_i2c0_lock(void)
{
    smart_shed_i2c0_init();
    osMutexAcquire(g_i2c0_mutex, osWaitForever);
}

void smart_shed_i2c0_unlock(void)
{
    if (g_i2c0_mutex != NULL) {
        osMutexRelease(g_i2c0_mutex);
    }
}
