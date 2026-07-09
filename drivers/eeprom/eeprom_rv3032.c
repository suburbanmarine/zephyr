/*
 * Copyright (c) 2026 Schloss Automation LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/mfd/rv3032.h>

#define DT_DRV_COMPAT microcrystal_rv3032_eeprom

LOG_MODULE_REGISTER(eeprom_rv3032, CONFIG_EEPROM_LOG_LEVEL);

struct eeprom_rv3032_config {
	const struct device *mfd;
};

static int eeprom_rv3032_read(const struct device *dev,
				off_t offset, void *data, size_t len)
{
	const struct eeprom_rv3032_config *config = dev->config;
	int err;

	if((offset + len) > RV3032_EEPROM_USER_SIZE) {
		return -EINVAL;
	}

	err = mfd_rv3032_enter_eerd(config->mfd);
	if (err) {
		LOG_ERR("Failed to enter EERD mode: %d", err);
		return err;
	}

	err = mfd_rv3032_eeprom_read_mult(config->mfd, RV3032_REG_EEPROM_USER0 + offset, data, len);
	if (err) {
		LOG_ERR("mfd_rv3032_eeprom_read_mult failed: %d", err);
		return err;
	}

	return 0;
}

static int eeprom_rv3032_write(const struct device *dev,
				 off_t offset, const void *data, size_t len)
{
	const struct eeprom_rv3032_config *config = dev->config;
	int err;

	if((offset + len) > RV3032_EEPROM_USER_SIZE) {
		return -EINVAL;
	}

	err = mfd_rv3032_enter_eerd(config->mfd);
	if (err) {
		LOG_ERR("Failed to enter EERD mode: %d", err);
		return err;
	}

	err = mfd_rv3032_eeprom_write_mult(config->mfd, RV3032_REG_EEPROM_USER0 + offset, data, len);
	if (err) {
		LOG_ERR("mfd_rv3032_eeprom_read_mult failed: %d", err);
		return err;
	}

	return 0;
}

static size_t eeprom_rv3032_size(const struct device *dev)
{
	return 32;
}

static DEVICE_API(eeprom, eeprom_rv3032_api) = {
	.read = eeprom_rv3032_read,
	.write = eeprom_rv3032_write,
	.size = eeprom_rv3032_size,
};

#define EEPROM_RV3032_INIT(inst)                                                                          \
                                                                                                   \
	static const struct eeprom_rv3032_config eeprom_rv3032_config_##inst = {                                 \
		.mfd = DEVICE_DT_GET(DT_INST_PARENT(inst)),                                        \
	};                                                                                         \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(inst, NULL, NULL, NULL,                       \
			      &eeprom_rv3032_config_##inst, POST_KERNEL, CONFIG_EEPROM_INIT_PRIORITY,        \
			      &eeprom_rv3032_api);

DT_INST_FOREACH_STATUS_OKAY(EEPROM_RV3032_INIT)
