#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/bitops.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include "BME280_SensorAPI/bme280_defs.h"
#include "BME280_SensorAPI/bme280.h"
#include "BME280_SensorAPI/common/common.h"

#define PRINT_INFO_DEBUG

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

/*Driver private data structure*/
struct bme280drv_data
{
	int total_devices;
};

struct bme280drv_data bme280_drv_data;

ssize_t bme280_humidity_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	int8_t rslt;
	uint32_t humidity_value;
	uint32_t period;
	struct bme280_dev *bme280_data = dev_get_drvdata(dev);
    
	/* Calculate measurement time in microseconds */
    rslt = bme280_cal_meas_delay(&period, bme280_data->settings);
    bme280_error_codes_print_result("bme280_cal_meas_delay", rslt);

    pr_info("\nHumidity calculation (Data displayed are compensated values)\n");
    pr_info("Measurement time : %lu us\n\n", (long unsigned int)period);

    humidity_value = get_pressure(period, bme280_data);
    pr_info("BME280 humidity value read: %d\n", humidity_value);

	return sprintf(buf,"%d\n",humidity_value);
}

ssize_t bme280_pressure_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	int8_t rslt;
	uint32_t pressure_value;
	uint32_t period;
	struct bme280_dev *bme280_data = dev_get_drvdata(dev);
    
	/* Calculate measurement time in microseconds */
    rslt = bme280_cal_meas_delay(&period, bme280_data->settings);
    bme280_error_codes_print_result("bme280_cal_meas_delay", rslt);

    pr_info("\nPressure calculation (Data displayed are compensated values)\n");
    pr_info("Measurement time : %lu us\n\n", (long unsigned int)period);

    pressure_value = get_pressure(period, bme280_data);
    pr_info("BME280 pressure value read: %d\n", pressure_value);

	return sprintf(buf,"%d\n",pressure_value);

}

ssize_t bme280_temperature_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	int8_t rslt;
	uint32_t temperature_value;
	uint32_t period;
	struct bme280_dev *bme280_data = dev_get_drvdata(dev);
    
	/* Calculate measurement time in microseconds */
    rslt = bme280_cal_meas_delay(&period, bme280_data->settings);
    bme280_error_codes_print_result("bme280_cal_meas_delay", rslt);

    pr_info("\nTemperature calculation (Data displayed are compensated values)\n");
    pr_info("Measurement time : %lu us\n\n", (long unsigned int)period);

    temperature_value = get_pressure(period, bme280_data);
    pr_info("BME280 temperature value read: %d\n", temperature_value);
	
	return sprintf(buf,"%d\n",temperature_value);

}

static DEVICE_ATTR_RO(bme280_humidity);
static DEVICE_ATTR_RO(bme280_pressure);
static DEVICE_ATTR_RO(bme280_temperature);

static struct attribute *bme280_attrs[] = 
{
	&dev_attr_bme280_humidity.attr,
	&dev_attr_bme280_pressure.attr,
	&dev_attr_bme280_temperature.attr,
	NULL
};

static struct attribute_group bme280_attr_group =
{
	.attrs = bme280_attrs
};

static const struct attribute_group *bme280_attr_groups[] = 
{
	&bme280_attr_group,
	NULL

};


static int bme280_probe(struct spi_device *spi_dev)
{
	int ret;
    int8_t rslt;
	const char *name;
	struct device *hwmon_dev;
    struct bme280_dev *bme280_data;

	/*parent device node*/
 	struct device_node *parent = spi_dev->dev.of_node;
	struct bme280_settings *settings;

    bme280_drv_data.total_devices++;
    pr_info("Driver's total devices  = %d\n",bme280_drv_data.total_devices);


	bme280_data = devm_kzalloc(&spi_dev->dev, sizeof(*bme280_data), GFP_KERNEL);
	if(!bme280_data){

		dev_err(&spi_dev->dev, "Cannot allocate memory\n");
		return -ENOMEM;

	};

	/*Allocate memory for bme280 settings*/
	settings = devm_kzalloc(&spi_dev->dev, sizeof(*settings), GFP_KERNEL);
	if(!settings){

		dev_err(&spi_dev->dev, "Cannot allocate memory\n");
		return -ENOMEM;		

	}

	/*Store BME280 settings in BM280 privete data*/
	bme280_data->settings= settings;  

	if(of_property_read_string(parent,"label",&name) )
	{

		dev_warn(&spi_dev->dev,"Missing label information\n");
		snprintf(bme280_data->label, sizeof(bme280_data->label),"unknlabel%d",bme280_drv_data.total_devices);

	}else{

		strcpy(bme280_data->label,name);
		dev_info(&spi_dev->dev,"Sensor label = %s\n",bme280_data->label);

	}

	bme280_data->spi_dev = spi_dev;
	spi_dev->bits_per_word = 8;
	spi_dev->mode = SPI_MODE_3;

	dev_info(&spi_dev->dev,"SPI Bus configuration\n");
        dev_info(&spi_dev->dev,"	max_speed_hz: %u\n", spi_dev->max_speed_hz);
        dev_info(&spi_dev->dev,"	bits_per_word: %u\n", spi_dev->bits_per_word);
        dev_info(&spi_dev->dev,"	mode: %u\n", spi_dev->mode);
        dev_info(&spi_dev->dev,"	modalias: %s\n", spi_dev->modalias);
	dev_info(&spi_dev->dev,"Setting up the SPI Bus.......\n");

	ret = spi_setup(spi_dev);
	if(ret < 0) {

		dev_err(&spi_dev->dev,"Failed to set up the SPI Bus\n");
		return ret;

	}

	/* Interface selection is to be updated as parameter
	* For I2C :  BME280_I2C_INTF
	* For SPI :  BME280_SPI_INTF
	*/

	rslt = bme280_interface_selection(bme280_data, BME280_SPI_INTF);
	bme280_error_codes_print_result("bme280_interface_selection", rslt);

	rslt = bme280_init(bme280_data);
	bme280_error_codes_print_result("bme280_init", rslt);

	/* Always read the current settings before writing, especially when all the configuration is not modified */
    //rslt = bme280_get_sensor_settings(settings, bme280_data);
    //bme280_error_codes_print_result("bme280_get_sensor_settings", rslt);

	/* Configuring the over-sampling rate, filter coefficient and standby time */
    /* Overwrite the desired settings */
    settings->filter = BME280_FILTER_COEFF_2;

    /* Over-sampling rate for humidity, temperature and pressure */
    settings->osr_h = BME280_OVERSAMPLING_1X;
    settings->osr_p = BME280_OVERSAMPLING_1X;
    settings->osr_t = BME280_OVERSAMPLING_1X;

    /* Setting the standby time */
    settings->standby_time = BME280_STANDBY_TIME_0_5_MS;

    //rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, settings, bme280_data);
    //bme280_error_codes_print_result("bme280_set_sensor_settings", rslt);

	hwmon_dev = devm_hwmon_device_register_with_groups(&spi_dev->dev,
							   spi_dev->modalias, 
							   bme280_data, bme280_attr_groups);

	if(IS_ERR(hwmon_dev)){
		dev_err(&spi_dev->dev,"Error in device_register \n");
		return PTR_ERR(hwmon_dev);
	}

	return 0;
}

struct of_device_id  bme280_device_match[] = 
{
	{.compatible = "org,bme280sensor"},
	{ }
};

static struct spi_driver bme280_driver = {
	.driver = {
		.name = "bme280sensor",
		.of_match_table = of_match_ptr(bme280_device_match)
	},
	.probe = bme280_probe
};

module_spi_driver(bme280_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JESUS HUMBERTO ONTIVEROS MAYORQUIN <humbertoontiveros08@gmail.com>");
MODULE_DESCRIPTION("BME280 digital temperature, pressure & humidity sensor driver");
