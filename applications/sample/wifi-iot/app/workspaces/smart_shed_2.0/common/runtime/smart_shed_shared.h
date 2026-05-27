#ifndef SMART_SHED_SHARED_H
#define SMART_SHED_SHARED_H

extern int fan_level;
extern int led_level;
extern int water_pump_level;

void smart_shed_i2c0_init(void);
void smart_shed_i2c0_lock(void);
void smart_shed_i2c0_unlock(void);

#endif
