#include "bme680_hal.h"

static struct bme68x_dev       dev;
static struct bme68x_conf      conf;
static struct bme68x_heatr_conf heatr;
static uint8_t dev_addr = BME68X_I2C_ADDR_LOW;  /* SDO=GND → 0x76 */

static int8_t bme_read(uint8_t reg, uint8_t *data, uint32_t len, void *ptr)
{
    uint8_t addr = *(uint8_t *)ptr;
    if (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(addr << 1), reg,
                          I2C_MEMADD_SIZE_8BIT, data, (uint16_t)len, 100) != HAL_OK)
        return BME68X_E_COM_FAIL;
    return BME68X_OK;
}

static int8_t bme_write(uint8_t reg, const uint8_t *data, uint32_t len, void *ptr)
{
    uint8_t addr = *(uint8_t *)ptr;
    if (HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(addr << 1), reg,
                           I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, (uint16_t)len, 100) != HAL_OK)
        return BME68X_E_COM_FAIL;
    return BME68X_OK;
}

static void bme_delay_us(uint32_t us, void *ptr)
{
    (void)ptr;
    HAL_Delay((us + 999U) / 1000U);
}

HAL_StatusTypeDef BME680_Init(void)
{
    dev.intf     = BME68X_I2C_INTF;
    dev.read     = bme_read;
    dev.write    = bme_write;
    dev.delay_us = bme_delay_us;
    dev.intf_ptr = &dev_addr;

    if (bme68x_init(&dev) != BME68X_OK) return HAL_ERROR;

    conf.filter  = BME68X_FILTER_OFF;
    conf.odr     = BME68X_ODR_NONE;
    conf.os_hum  = BME68X_OS_1X;
    conf.os_pres = BME68X_OS_4X;
    conf.os_temp = BME68X_OS_2X;
    if (bme68x_set_conf(&conf, &dev) != BME68X_OK) return HAL_ERROR;

    heatr.enable     = BME68X_ENABLE;
    heatr.heatr_temp = 300;
    heatr.heatr_dur  = 100;
    if (bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr, &dev) != BME68X_OK)
        return HAL_ERROR;

    return HAL_OK;
}

HAL_StatusTypeDef BME680_ReadForced(BME680_Result *out)
{
    if (bme68x_set_op_mode(BME68X_FORCED_MODE, &dev) != BME68X_OK) return HAL_ERROR;

    uint32_t del_us = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev)
                    + (uint32_t)heatr.heatr_dur * 1000U;
    bme_delay_us(del_us, NULL);

    struct bme68x_data data;
    uint8_t n = 0;
    if (bme68x_get_data(BME68X_FORCED_MODE, &data, &n, &dev) != BME68X_OK || n == 0)
        return HAL_ERROR;

    out->temperature    = data.temperature;
    out->humidity       = data.humidity;
    out->pressure       = data.pressure / 100.0f;
    out->gas_resistance = data.gas_resistance;
    return HAL_OK;
}
