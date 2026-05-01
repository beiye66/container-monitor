#include "mpu6050.h"

#define MPU_ADDR        (0x68 << 1)  /* AD0 = GND → 7-bit 0x68, HAL needs 8-bit */
#define REG_PWR_MGMT_1  0x6B
#define REG_CONFIG      0x1A
#define REG_GYRO_CFG    0x1B
#define REG_ACCEL_CFG   0x1C
#define REG_ACCEL_XOUT  0x3B

static HAL_StatusTypeDef mpu_write(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDR, buf, 2, 10);
}

HAL_StatusTypeDef MPU6050_Init(void)
{
    HAL_StatusTypeDef s;
    s = mpu_write(REG_PWR_MGMT_1, 0x00);  /* 唤醒，清除 sleep 位 */
    if (s != HAL_OK) return s;
    HAL_Delay(10);
    s = mpu_write(REG_CONFIG,    0x04);   /* DLPF ~21 Hz，避免 50Hz 混叠 */
    if (s != HAL_OK) return s;
    s = mpu_write(REG_GYRO_CFG,  0x18);  /* 陀螺仪 ±2000 °/s */
    if (s != HAL_OK) return s;
    s = mpu_write(REG_ACCEL_CFG, 0x10);  /* 加速度计 ±8 g，满足碰撞检测 */
    return s;
}

HAL_StatusTypeDef MPU6050_Read(int16_t *ax, int16_t *ay, int16_t *az,
                               int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t buf[14];
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, REG_ACCEL_XOUT,
                                            I2C_MEMADD_SIZE_8BIT, buf, 14, 20);
    if (s != HAL_OK) return s;

    *ax = (int16_t)((buf[0]  << 8) | buf[1]);
    *ay = (int16_t)((buf[2]  << 8) | buf[3]);
    *az = (int16_t)((buf[4]  << 8) | buf[5]);
    /* buf[6..7] = 片内温度，跳过 */
    *gx = (int16_t)((buf[8]  << 8) | buf[9]);
    *gy = (int16_t)((buf[10] << 8) | buf[11]);
    *gz = (int16_t)((buf[12] << 8) | buf[13]);
    return HAL_OK;
}
