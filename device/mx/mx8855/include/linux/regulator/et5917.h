/* SPDX-License-Identifier: GPL-2.0 */
/*
 * et5917.h - Regulator driver for the ETEK ET5917SX
 *
 * Copyright (c) 2024 ETEK Microcircuits Co., Ltd Jiangsu
 *
 */

#ifndef __LINUX_REGULATOR_ET5917SX_H
#define __LINUX_REGULATOR_ET5917SX_H
#include <linux/platform_device.h>
#include <linux/regmap.h>

struct et5917sx_dev {
	struct device *dev;
	struct i2c_client *i2c;
	struct mutex i2c_lock;

	int type;
	u8 rev_num; /* pmic Rev */
	bool wakeup;

	int	et5917sx_irq;

	struct et5917sx_platform_data *pdata;
};

struct et5917sx_regulator_data {
	int id;
	struct regulator_init_data *initdata;
	struct device_node *reg_node;
};

struct et5917sx_platform_data {
	bool wakeup;
	bool need_self_recovery;
	int num_regulators;
	int num_rdata;
	struct	et5917sx_regulator_data *regulators;
	int	device_type;
	int et5917sx_irq_gpio;
	u32 et5917sx_int_level;
	u32 et5917sx_int_outmode;
};


/* ET5917SX registers */
/* Slave Addr : 0xC2 */
enum ET5917SX_reg {
	ET5917SX_REG_CHIP_ID, // 0x00
	ET5917SX_REG_VER_ID, // 0x01
	ET5917SX_REG_LDO_ILIMIT,
	ET5917SX_REG_LDO_EN,
	ET5917SX_REG_LDO1_VSET,
	ET5917SX_REG_LDO2_VSET,
	ET5917SX_REG_LDO3_VSET,
	ET5917SX_REG_LDO4_VSET,
	ET5917SX_REG_LDO5_VSET,
	ET5917SX_REG_LDO6_VSET,
	ET5917SX_REG_LDO7_VSET,
	ET5917SX_REG_LDO12_SEQ,
	ET5917SX_REG_LDO34_SEQ,
	ET5917SX_REG_LDO56_SEQ,
	ET5917SX_REG_LDO7_SEQ,
	ET5917SX_REG_SEQ_CTR,
	ET5917SX_REG_LDO_DIS,
	ET5917SX_REG_RESET,
	ET5917SX_REG_I2C_ADDR,
	ET5917SX_REG_REV1,
	ET5917SX_REG_REV2,
	ET5917SX_REG_UVP_INT,
	ET5917SX_REG_OCP_INT,
	ET5917SX_REG_TSD_UVLO_INT,
	ET5917SX_REG_UVP_STAU,
	ET5917SX_REG_OCP_STAU,
	ET5917SX_REG_TSD_UVLO_STAU,
	ET5917SX_REG_SUSD_STAU,
	ET5917SX_REG_UVP_INTMA,
	ET5917SX_REG_OCP_INTMA,
	ET5917SX_REG_TSD_UVLO_INTMA,
};

/* ET5917SX regulator ids */
enum ET5917SX_regulators {
	ET5917SX_LDO1,
	ET5917SX_LDO2,
	ET5917SX_LDO3,
	ET5917SX_LDO4,
	ET5917SX_LDO5,
	ET5917SX_LDO6,
	ET5917SX_LDO7,
	ET5917SX_LDO_MAX,
};


#define ET5917SX_LDO_VSEL_MASK	0xFF
/* Ramp delay in uV/us */
// 1.2v * 0.95 / 200 us
#define ET5917SX_RAMP_DELAY1		5700
// 2.8v * 0.95 / 200 us
#define ET5917SX_RAMP_DELAY2		13300

#define ET5917SX_ENABLE_TIME_LDO		50

#define ET5917SX_REGULATOR_MAX (ET5917SX_LDO_MAX)

#define ET5917SX_EN_VSYS_BIT	BIT(7)

#endif /*  __LINUX_ETEK_ET5917SX_H */
