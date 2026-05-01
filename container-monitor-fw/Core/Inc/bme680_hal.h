#ifndef __BME680_HAL_H__
#define __BME680_HAL_H__

#include "bme68x.h"
#include "i2c.h"

typedef struct {
    float    temperature;    /* °C  */
    float    humidity;       /* %RH */
    float    pressure;       /* hPa */
    uint32_t gas_resistance; /* Ω   */
} BME680_Result;

HAL_StatusTypeDef BME680_Init(void);
HAL_StatusTypeDef BME680_ReadForced(BME680_Result *out);

#endif /* __BME680_HAL_H__ */
