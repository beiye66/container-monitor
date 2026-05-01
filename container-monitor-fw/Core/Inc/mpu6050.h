#ifndef __MPU6050_H__
#define __MPU6050_H__

#include "i2c.h"

HAL_StatusTypeDef MPU6050_Init(void);
HAL_StatusTypeDef MPU6050_Read(int16_t *ax, int16_t *ay, int16_t *az,
                               int16_t *gx, int16_t *gy, int16_t *gz);

#endif /* __MPU6050_H__ */
