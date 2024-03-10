/**
 * Copyright (C) 2020 Bosch Sensortec GmbH. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//#include <stdint.h>
//#include <stdlib.h>
//#include <stdio.h>

//#include "coines.h"
#include "../bme280.h"
#include "common.h"
#include <linux/delay.h>

/******************************************************************************/
/*!                               Macros                                      */

#define BME280_SHUTTLE_ID  UINT8_C(0x33)

/******************************************************************************/
/*!                Static variable definition                                 */

/*! Variable that holds the I2C device address or SPI chip selection */
static uint8_t dev_addr;

/******************************************************************************/
/*!                User interface functions                                   */

/*!
 * I2C read function map to COINES platform
 */
BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
   // dev_addr = *(uint8_t*)intf_ptr;

   // return coines_read_i2c(COINES_I2C_BUS_0, dev_addr, reg_addr, reg_data, (uint16_t)length);
	return 0;
}

/*!
 * I2C write function map to COINES platform
 */
BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
	dev_addr = *(uint8_t*)intf_ptr;

	 //return coines_write_i2c(COINES_I2C_BUS_0, dev_addr, reg_addr, (uint8_t *)reg_data, (uint16_t)length);

	spi_write(struct spi_device *spi, const void *buf, size_t len);
        return 0;
}

/*!
 * SPI read function map to COINES platform
 */
BME280_INTF_RET_TYPE bme280_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
	dev_addr = *(uint8_t*)intf_ptr;

    //return coines_read_spi(COINES_SPI_BUS_0, dev_addr, reg_addr, reg_data, (uint16_t)length);

	spi_read(struct spi_device *spi, void *buf, size_t len);
        return 0;
}

/*!
 * SPI write function map to COINES platform
 */
BME280_INTF_RET_TYPE bme280_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    //dev_addr = *(uint8_t*)intf_ptr;

    //return coines_write_spi(COINES_SPI_BUS_0, dev_addr, reg_addr, (uint8_t *)reg_data, (uint16_t)length);
        return 0;
}

/*!
 * Delay function map to COINES platform
 */
void bme280_delay_us(uint32_t period, void *intf_ptr)
{
	//coines_delay_usec(period);
	udelay(period);
}

/*!
 *  @brief Prints the execution status of the APIs.
 */
void bme280_error_codes_print_result(const char api_name[], int8_t rslt)
{
    if (rslt != BME280_OK)
    {
        pr_err("%s\t", api_name);

        switch (rslt)
        {
            case BME280_E_NULL_PTR:
                pr_err("Error [%d] : Null pointer error.", rslt);
                pr_err(
                    "It occurs when the user tries to assign value (not address) to a pointer, which has been initialized to NULL.\r\n");
                break;

            case BME280_E_COMM_FAIL:
                pr_err("Error [%d] : Communication failure error.", rslt);
                pr_err(
                    "It occurs due to read/write operation failure and also due to power failure during communication\r\n");
                break;

            case BME280_E_DEV_NOT_FOUND:
                pr_err("Error [%d] : Device not found error. It occurs when the device chip id is incorrectly read\r\n",
                       rslt);
                break;

            case BME280_E_INVALID_LEN:
                pr_err("Error [%d] : Invalid length error. It occurs when write is done with invalid length\r\n", rslt);
                break;

            default:
                pr_err("Error [%d] : Unknown error code\r\n", rslt);
                break;
        }
    }
}

/*!
 *  @brief Function to select the interface between SPI and I2C.
 */
int8_t bme280_interface_selection(struct bme280_dev *dev, uint8_t intf)
{
    int8_t rslt = BME280_OK;
    if (dev != NULL)
    {
        /* Bus configuration : I2C */
        if (intf == BME280_I2C_INTF)
        {
            pr_info("I2C Interface\n");

            dev_addr = BME280_I2C_ADDR_PRIM;
            dev->read = bme280_i2c_read;
            dev->write = bme280_i2c_write;
            dev->intf = BME280_I2C_INTF;
        }
        /* Bus configuration : SPI */
        else if (intf == BME280_SPI_INTF)
        {
            pr_info("SPI Interface\n");

            //dev_addr = COINES_SHUTTLE_PIN_7;
            dev->read = bme280_spi_read;
            dev->write = bme280_spi_write;
            dev->intf = BME280_SPI_INTF;
        }

        /* Holds the I2C device addr or SPI chip selection */
        dev->intf_ptr = &dev_addr;

        /* Configure delay in microseconds */
        dev->delay_us = bme280_delay_us;

        //coines_delay_msec(100);

        //coines_set_shuttleboard_vdd_vddio_config(3300, 3300);

        //coines_delay_msec(100);
    }
    else
    {
        rslt = BME280_E_NULL_PTR;
    }

    return rslt;
}

