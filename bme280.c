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


#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__


/*Device private data structure*/
struct bme280data {
	char label[20];
	struct spi_device *spi_dev;
};

ssize_t bme280_humidity_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return 0;
}

ssize_t bme280_pressure_show(struct device *dev, struct device_attribute *attr,char *buf)
{
        return 0;
}

ssize_t bme280_temperature_show(struct device *dev, struct device_attribute *attr,char *buf)
{
        return 0;
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

int bme280_remove(struct spi_device *spi_dev)
{
	return 0;

}

static int bme280_probe(struct spi_device *spi_dev)
{
	struct bme280data *bme280_data;

	const char *name;
	int i = 0;
	int ret;
	struct device *hwmon_dev;

        int8_t rslt;
        struct bme280_dev dev;



	/*parent device node*/
 	struct device_node *parent = spi_dev->dev.of_node;


	bme280_data = devm_kzalloc(&spi_dev->dev, sizeof(*bme280_data), GFP_KERNEL);
	if(!bme280_data){

		dev_err(&spi_dev->dev, "Cannot allocate memory\n");
		return -ENOMEM;

	};

	if(of_property_read_string(parent,"label",&name) )
	{

		dev_warn(&spi_dev->dev,"Missing label information\n");
		snprintf(bme280_data->label, sizeof(bme280_data->label),"unknlabel%d",i);

	}else{

		strcpy(bme280_data->label,name);
		dev_info(&spi_dev->dev,"Sensor label = %s\n",bme280_data->label);

	}

	bme280_data->spi_dev = spi_dev;
	spi_dev->bits_per_word = 8;

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

	/*
	* make some  init here using  the bme280 api 
	* to condigure the sensor
	*
	*/


	/* Interface selection is to be updated as parameter
	* For I2C :  BME280_I2C_INTF
	* For SPI :  BME280_SPI_INTF
	*/

	rslt = bme280_interface_selection(&dev, BME280_I2C_INTF);
	bme280_error_codes_print_result("bme280_interface_selection", rslt);

	rslt = bme280_init(&dev);
	bme280_error_codes_print_result("bme280_init", rslt);

	hwmon_dev = devm_hwmon_device_register_with_groups(&spi_dev->dev,
							   spi_dev->modalias,
							   bme280_data, bme280_attr_groups);

	return PTR_ERR_OR_ZERO(hwmon_dev);
}

struct of_device_id  bme280_device_match[] = 
{
	
{.compatible = "org,bme280"},
	{ }
};

static struct spi_driver bme280_driver = {
	.driver = {
		.name = "bme280",
		.of_match_table = of_match_ptr(bme280_device_match)
	},
	.probe = bme280_probe,
	.remove = bme280_remove
};

module_spi_driver(bme280_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JESUS HUMBERTO ONTIVEROS MAYORQUIN <humbertoontiveros08@gmail.com>");
MODULE_DESCRIPTION("BME280 digital temperature, pressure & humidity sensor driver");
