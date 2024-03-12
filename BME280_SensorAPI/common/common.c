/**
 * Copyright (C) 2020 Bosch Sensortec GmbH. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../bme280.h"
#include "common.h"
#include <linux/delay.h>
#include <linux/spi/spi.h>

/******************************************************************************/
/*!                               Macros                                      */

#define BME280_SHUTTLE_ID  UINT8_C(0x33)
#define SAMPLE_COUNT  UINT8_C(50)

/******************************************************************************/
/*!                Static variable definition                                 */

/*! Variable that holds the I2C device address or SPI chip selection */
static uint8_t dev_addr;

/******************************************************************************/
/*!                User interface functions                                   */

/*!
 * I2C read function map to COINES platform
 */
BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr, struct spi_device *spi_dev)
{
	return 0;
}

/*!
 * I2C write function map to COINES platform
 */
BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr, struct spi_device *spi_dev)
{
        return 0;
}

/*!
 * SPI read function map to COINES platform
 */
BME280_INTF_RET_TYPE bme280_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr, struct spi_device *spi_dev)
{
	//dev_addr = *(uint8_t*)intf_ptr;

	struct spi_message message;
	struct spi_transfer transfer;

	memset(&transfer, 0, sizeof(struct spi_transfer));
	transfer.tx_buf = kmalloc(length, GFP_KERNEL);
	transfer.rx_buf = reg_data;
	transfer.len = length;

	// Configura el registro de dirección para la lectura
	*((uint8_t *)transfer.tx_buf) = reg_addr | 0x80;

	spi_message_init(&message);
	spi_message_add_tail(&transfer, &message);

	if (spi_sync(spi_dev, &message) == 0) {
		kfree(transfer.tx_buf);
		return 0;
	} else {
		kfree(transfer.tx_buf);
		return -EIO;
	}
}

/*!
 * SPI write function map to COINES platform
 */
BME280_INTF_RET_TYPE bme280_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr, struct spi_device *spi_dev)
{
	//dev_addr = *(uint8_t*)intf_ptr;
	int ret;
	struct spi_message message;
	struct spi_transfer transfer;

	/*
	*
	*Assuming that dev_addr is an SPI device, use spi_dev directly
	*dev_addr = spi_dev->chip_select;
	*
	*/

	/* Initialize the spi_transfer structure*/
	memset(&transfer, 0, sizeof(struct spi_transfer));
	transfer.tx_buf = kmalloc(length + 1, GFP_KERNEL);  // Allocate memory for reg_addr + reg_data
	if (!transfer.tx_buf) {
		return -ENOMEM;  // Memory allocation failure
	}

	/* Set the register address as the first byte*/
	*((uint8_t *)transfer.tx_buf) = reg_addr;
	memcpy((void *)(uintptr_t)(transfer.tx_buf + 1), reg_data, length);  // Copy reg_data after reg_addr
	transfer.len = length + 1;

	/* Initialize the spi_message structure and add the transfer*/
	spi_message_init(&message);
	spi_message_add_tail(&transfer, &message);

	/* Perform the SPI transfer*/
	ret = spi_sync(spi_dev, &message);

	/* Free allocated memory*/
	kfree(transfer.tx_buf);

	return ret;
}

/*!
 * Delay function map to COINES platform
 */
void bme280_delay_us(uint32_t period, void *intf_ptr)
{
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

/*!
 *  @brief This internal API is used to get compensated humidity data.
 */
int8_t get_humidity(uint32_t period, struct bme280_dev *dev)
{
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < SAMPLE_COUNT)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, dev);
        bme280_error_codes_print_result("bme280_get_regs", rslt);

        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            dev->delay_us(period, dev->intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_HUM, &comp_data, dev);
            bme280_error_codes_print_result("bme280_get_sensor_data", rslt);

#ifndef BME280_DOUBLE_ENABLE
            comp_data.humidity = comp_data.humidity / 1000;
#endif

#ifdef BME280_DOUBLE_ENABLE
            pr_info("Humidity[%d]:   %lf %%RH\n", idx, comp_data.humidity);
#else
            pr_info("Humidity[%d]:   %lu %%RH\n", idx, (long unsigned int)comp_data.humidity);
#endif
            idx++;
        }
    }

    return rslt;
}

/*!
 *  @brief This internal API is used to get compensated pressure data.
 */
int8_t get_pressure(uint32_t period, struct bme280_dev *dev)
{
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < SAMPLE_COUNT)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, dev);
        bme280_error_codes_print_result("bme280_get_regs", rslt);

        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            dev->delay_us(period, dev->intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_PRESS, &comp_data, dev);
            bme280_error_codes_print_result("bme280_get_sensor_data", rslt);

#ifdef BME280_64BIT_ENABLE
            comp_data.pressure = comp_data.pressure / 100;
#endif

#ifdef BME280_DOUBLE_ENABLE
            pr_info("Pressure[%d]:  %lf Pa\n", idx, comp_data.pressure);
#else
            pr_info("Pressure[%d]:   %lu Pa\n", idx, (long unsigned int)comp_data.pressure);
#endif
            idx++;
        }
    }

    return rslt;
}

/*!
 *  @brief This internal API is used to get compensated temperature data.
 */
int8_t get_temperature(uint32_t period, struct bme280_dev *dev)
{
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < SAMPLE_COUNT)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, dev);
        bme280_error_codes_print_result("bme280_get_regs", rslt);

        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            dev->delay_us(period, dev->intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_TEMP, &comp_data, dev);
            bme280_error_codes_print_result("bme280_get_sensor_data", rslt);

#ifndef BME280_DOUBLE_ENABLE
            comp_data.temperature = comp_data.temperature / 100;
#endif

#ifdef BME280_DOUBLE_ENABLE
            pr_info("Temperature[%d]:   %lf deg C\n", idx, comp_data.temperature);
#else
            pr_info("Temperature[%d]:   %ld deg C\n", idx, (long int)comp_data.temperature);
#endif
            idx++;
        }
    }

    return rslt;
}
