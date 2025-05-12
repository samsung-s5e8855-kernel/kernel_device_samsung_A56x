// SPDX-License-Identifier: GPL-2.0
/*
 * et5917.c - Regulator driver for the ETEK ET5917SX
 *
 * Copyright (c) 2024 ETEK Microcircuits Co., Ltd Jiangsu
 *
 */

#include <linux/device.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include <linux/of_gpio.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/et5917.h>
#include <linux/regulator/of_regulator.h>


#if IS_ENABLED(CONFIG_DRV_SAMSUNG)
#include <linux/sec_class.h>
#else

#if defined(_SUPPORT_SYSFS_INTERFACE)
struct pmic_device_attribute {
	struct device_attribute dev_attr;
};

#define PMIC_ATTR(_name, _mode, _show, _store)	\
	{ .dev_attr = __ATTR(_name, _mode, _show, _store) }

static struct class *pmic_class;
static atomic_t pmic_dev;

#endif /*end of #if defined(_SUPPORT_SYSFS_INTERFACE)*/
#endif /*end of #if IS_ENABLED(CONFIG_DRV_SAMSUNG)*/

struct et5917sx_data {
	struct et5917sx_dev *iodev;
	int num_regulators;
	struct regulator_dev *rdev[ET5917SX_REGULATOR_MAX];

	struct workqueue_struct *wq;
	struct work_struct work;
	bool need_self_recovery;

#if IS_ENABLED(CONFIG_DRV_SAMSUNG) || defined(_SUPPORT_SYSFS_INTERFACE)
	u8 read_addr;
	u8 read_val;
	struct device *dev;
#endif
};

// 0x3d ~ 0xe0  VOUT = 0.496V + [(d − 61) x 8mV]
static const struct linear_range et5917sx_ldo12_range[] = {
	REGULATOR_LINEAR_RANGE(496000, 0x3d, 0xe0, 8000),
};

// 0x00 ~ 0xff   VOUT = 1.372V + [(d - 0) x 8mV]
static const struct linear_range et5917sx_ldo37_range[] = {
	REGULATOR_LINEAR_RANGE(1372000, 0x00, 0xff, 8000),
};

#define ET5917SX_LDO_CURRENT_COUNT	2
static const unsigned int et5917sx_ldo_current[ET5917SX_REGULATOR_MAX][ET5917SX_LDO_CURRENT_COUNT] = {
	{ 1900000, 2500000 },
	{ 1900000, 2500000 },
	{ 750000, 950000 },
	{ 750000, 950000 },
	{ 750000, 950000 },
	{ 750000, 950000 },
	{ 750000, 950000 }
};

int et5917sx_read_reg(struct i2c_client *i2c, u8 reg, u8 *dest)
{
	struct et5917sx_data *info = i2c_get_clientdata(i2c);
	struct et5917sx_dev *et5917sx = info->iodev;
	int ret;

	mutex_lock(&et5917sx->i2c_lock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	mutex_unlock(&et5917sx->i2c_lock);
	if (ret < 0) {
		dev_err(&i2c->dev, "%s: reg(0x%02x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}

	ret &= 0xff;
	*dest = ret;
	return 0;
}

int et5917sx_bulk_read(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct et5917sx_data *info = i2c_get_clientdata(i2c);
	struct et5917sx_dev *et5917sx = info->iodev;
	int ret;

	mutex_lock(&et5917sx->i2c_lock);
	ret = i2c_smbus_read_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&et5917sx->i2c_lock);
	if (ret < 0)
		return ret;

	return 0;
}

int et5917sx_read_word(struct i2c_client *i2c, u8 reg)
{
	struct et5917sx_data *info = i2c_get_clientdata(i2c);
	struct et5917sx_dev *et5917sx = info->iodev;
	int ret;

	mutex_lock(&et5917sx->i2c_lock);
	ret = i2c_smbus_read_word_data(i2c, reg);
	mutex_unlock(&et5917sx->i2c_lock);
	if (ret < 0)
		return ret;

	return ret;
}

int et5917sx_write_reg(struct i2c_client *i2c, u8 reg, u8 value)
{
	struct et5917sx_data *info = i2c_get_clientdata(i2c);
	struct et5917sx_dev *et5917sx = info->iodev;
	int ret;

	mutex_lock(&et5917sx->i2c_lock);
	ret = i2c_smbus_write_byte_data(i2c, reg, value);
	mutex_unlock(&et5917sx->i2c_lock);
	if (ret < 0)
		dev_err(&i2c->dev, "%s: reg(0x%02hhx), ret(%d)\n", __func__, reg, ret);

	return ret;
}

int et5917sx_bulk_write(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct et5917sx_data *info = i2c_get_clientdata(i2c);
	struct et5917sx_dev *et5917sx = info->iodev;
	int ret;

	mutex_lock(&et5917sx->i2c_lock);
	ret = i2c_smbus_write_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&et5917sx->i2c_lock);
	if (ret < 0)
		return ret;

	return 0;
}

int et5917sx_update_reg(struct i2c_client *i2c, u8 reg, u8 val, u8 mask)
{
	struct et5917sx_data *info = i2c_get_clientdata(i2c);
	struct et5917sx_dev *et5917sx = info->iodev;
	int ret;
	u8 old_val, new_val;

	mutex_lock(&et5917sx->i2c_lock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret >= 0) {
		old_val = ret & 0xff;
		new_val = (val & mask) | (old_val & (~mask));
		ret = i2c_smbus_write_byte_data(i2c, reg, new_val);
	}
	mutex_unlock(&et5917sx->i2c_lock);

	return ret;
}

static int et5917sx_set_interrupt(struct i2c_client *i2c, u32 int_level_sel, u32 int_outmode_sel)
{
	int ret = 0;
	u8 val = 0;

	if (i2c) {
		ret = et5917sx_read_reg(i2c, ET5917SX_REG_I2C_ADDR, &val);
		if (ret < 0) {
			dev_err(&i2c->dev, "%s: fail to read I2C_ADDR\n", __func__);
			return ret;
		}
		dev_info(&i2c->dev, "%s: read I2C_ADDR %d 0x%x\n", __func__, ret, val);

		if (int_level_sel)
			val |= 0x80;
		else
			val &= ~0x80;

		if (int_outmode_sel)
			val |= 0x40;
		else
			val &= ~0x40;

		et5917sx_write_reg(i2c, ET5917SX_REG_I2C_ADDR, val);
		dev_info(&i2c->dev, "%s: write I2C_ADDR %d 0x%x\n", __func__, ret, val);
	}

	return 0;
}

static int et5917sx_enable(struct regulator_dev *rdev)
{
	struct et5917sx_data *info = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = info->iodev->i2c;

	return et5917sx_update_reg(i2c, rdev->desc->enable_reg,
					rdev->desc->enable_mask,
					rdev->desc->enable_mask);
}

static int et5917sx_disable_regmap(struct regulator_dev *rdev)
{
	struct et5917sx_data *info = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = info->iodev->i2c;

	return et5917sx_update_reg(i2c, rdev->desc->enable_reg,
					0, rdev->desc->enable_mask);
}

static int et5917sx_is_enabled_regmap(struct regulator_dev *rdev)
{
	struct et5917sx_data *info = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = info->iodev->i2c;
	int ret;
	u8 val;

	ret = et5917sx_read_reg(i2c, rdev->desc->enable_reg, &val);
	if (ret < 0)
		return ret;

	return (val & rdev->desc->enable_mask) != 0;
}

static int et5917sx_get_voltage_sel_regmap(struct regulator_dev *rdev)
{
	struct et5917sx_data *info = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = info->iodev->i2c;
	int ret;
	u8 val;

	ret = et5917sx_read_reg(i2c, rdev->desc->vsel_reg, &val);
	if (ret < 0)
		return ret;

	val &= rdev->desc->vsel_mask;

	return val;
}

static int et5917sx_set_voltage_sel_regmap(struct regulator_dev *rdev, unsigned int sel)
{
	struct et5917sx_data *info = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = info->iodev->i2c;
	int ret;

	ret = et5917sx_update_reg(i2c, rdev->desc->vsel_reg,
				sel, rdev->desc->vsel_mask);
	if (ret < 0)
		goto out;

	if (rdev->desc->apply_bit)
		ret = et5917sx_update_reg(i2c, rdev->desc->apply_reg,
					rdev->desc->apply_bit,
					rdev->desc->apply_bit);
	return ret;
out:
	dev_warn(&i2c->dev, "%s: failed to set voltage_sel_regmap\n", rdev->desc->name);
	return ret;
}

static int et5917sx_set_current_limit(struct regulator_dev *rdev, int min_uA, int max_uA)
{
	struct et5917sx_data *info = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = info->iodev->i2c;
	int i, sel = -1;

	if (rdev->desc->id < ET5917SX_LDO1 || rdev->desc->id >= ET5917SX_REGULATOR_MAX)
		return -EINVAL;

	for (i = ET5917SX_LDO_CURRENT_COUNT-1; i >= 0 ; i--) {
		if (min_uA <= et5917sx_ldo_current[rdev->desc->id][i] &&
				et5917sx_ldo_current[rdev->desc->id][i] <= max_uA) {
			sel = i;
			break;
		}
	}

	if (sel < 0)
		return -EINVAL;

	sel <<= ffs(rdev->desc->enable_mask) - 1;

	return et5917sx_update_reg(i2c, ET5917SX_REG_LDO_ILIMIT, sel, rdev->desc->enable_mask);
}


static int et5917sx_get_current_limit(struct regulator_dev *rdev)
{
	struct et5917sx_data *info = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = info->iodev->i2c;
	u8 val = 0;
	int ret = 0;

	ret = et5917sx_read_reg(i2c, ET5917SX_REG_LDO_ILIMIT, &val);
	if (ret < 0)
		return ret;

	val &= rdev->desc->enable_mask;
	val >>= ffs(rdev->desc->enable_mask) - 1;
	if (val < ET5917SX_LDO_CURRENT_COUNT &&
		(rdev->desc->id >= ET5917SX_LDO1 && rdev->desc->id < ET5917SX_REGULATOR_MAX))
		return et5917sx_ldo_current[rdev->desc->id][val];

	return -EINVAL;
}

static const struct regulator_ops et5917sx_ldo_ops = {
	.list_voltage		= regulator_list_voltage_linear_range,
	.is_enabled			= et5917sx_is_enabled_regmap,
	.enable				= et5917sx_enable,
	.disable			= et5917sx_disable_regmap,
	.get_voltage_sel	= et5917sx_get_voltage_sel_regmap,
	.set_voltage_sel	= et5917sx_set_voltage_sel_regmap,
	.set_voltage_time_sel	= regulator_set_voltage_time_sel,
	.set_current_limit		= et5917sx_set_current_limit,
	.get_current_limit		= et5917sx_get_current_limit,
};

#define _LDO(macro)	ET5917SX_LDO##macro
#define _REG(ctrl)	ET5917SX_REG##ctrl
#define _ldo_ops(num)	et5917sx_ldo_ops##num


#define LDO_DESC(_name, _id, _ops, d, r, v, e)	{	\
	.name		= _name,				\
	.id		= _id,					\
	.ops		= _ops,					\
	.type		= REGULATOR_VOLTAGE,			\
	.owner		= THIS_MODULE,				\
	.ramp_delay = d,							\
	.linear_ranges = r,					\
	.n_linear_ranges = ARRAY_SIZE(r),		\
	.vsel_reg	= v,					\
	.vsel_mask	= ET5917SX_LDO_VSEL_MASK,		\
	.enable_reg	= ET5917SX_REG_LDO_EN,			\
	.enable_mask	= 0x01<<e,		\
	.enable_time	= ET5917SX_ENABLE_TIME_LDO	\
}


static struct regulator_desc regulators[ET5917SX_REGULATOR_MAX] = {
	/* name, id, ops, min_uv, uV_step, vsel_reg, enable_reg */
	LDO_DESC("et5917-ldo1", _LDO(1), &_ldo_ops(), ET5917SX_RAMP_DELAY1, et5917sx_ldo12_range,
		_REG(_LDO1_VSET), _LDO(1)),
	LDO_DESC("et5917-ldo2", _LDO(2), &_ldo_ops(), ET5917SX_RAMP_DELAY1, et5917sx_ldo12_range,
		_REG(_LDO2_VSET), _LDO(2)),
	LDO_DESC("et5917-ldo3", _LDO(3), &_ldo_ops(), ET5917SX_RAMP_DELAY2, et5917sx_ldo37_range,
		_REG(_LDO3_VSET), _LDO(3)),
	LDO_DESC("et5917-ldo4", _LDO(4), &_ldo_ops(), ET5917SX_RAMP_DELAY2, et5917sx_ldo37_range,
		_REG(_LDO4_VSET), _LDO(4)),
	LDO_DESC("et5917-ldo5", _LDO(5), &_ldo_ops(), ET5917SX_RAMP_DELAY2, et5917sx_ldo37_range,
		_REG(_LDO5_VSET), _LDO(5)),
	LDO_DESC("et5917-ldo6", _LDO(6), &_ldo_ops(), ET5917SX_RAMP_DELAY2, et5917sx_ldo37_range,
		_REG(_LDO6_VSET), _LDO(6)),
	LDO_DESC("et5917-ldo7", _LDO(7), &_ldo_ops(), ET5917SX_RAMP_DELAY2, et5917sx_ldo37_range,
		_REG(_LDO7_VSET), _LDO(7))
};

#if IS_ENABLED(CONFIG_SEC_FACTORY)
/*
 * Recovery logic for et5917sx detach test
 */

bool regulator_should_be_enabled(struct regulator_dev *rdev)
{
	return (rdev->constraints->always_on) || (rdev->use_count > 0);
}

int et5917sx_need_self_recovery(struct et5917sx_data *et5917sx)
{
	struct regulator_dev *rdev;
	int i, ret = 0;

	for (i = 0; i < et5917sx->num_regulators; i++) {
		rdev = et5917sx->rdev[i];
		if (!rdev)
			continue;

		if (!regulator_should_be_enabled(rdev))
			continue;

		if (!et5917sx_is_enabled_regmap(rdev)) {
			pr_info("%s: %s\n", __func__, rdev->desc->name);
			ret = 1;
		}
	}

	return ret;
}

int et5917sx_recovery_voltage(struct regulator_dev *rdev)
{
	int ret = -1;
	unsigned int vol, reg;

	// Get and calculate voltage from regulator framework
	vol = (rdev->constraints->min_uV - rdev->desc->min_uV) / rdev->desc->uV_step;
	reg = et5917sx_get_voltage_sel_regmap(rdev);

	// Set proper voltage according to regulator type
	if (rdev->desc->vsel_mask == ET5917SX_LDO_VSEL_MASK)
		ret = et5917sx_set_voltage_sel_regmap(rdev, vol);

	return ret;
}

int et5917sx_recovery(struct et5917sx_data *et5917sx)
{
	struct regulator_dev *rdev;
	int i, ret = 0;

	if (!et5917sx) {
		pr_info("%s: There is no local rdev data\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < et5917sx->num_regulators; i++) {
		rdev = et5917sx->rdev[i];
		if (!rdev)
			continue;

		pr_info("%s: %s: max(%d), min(%d), always_on(%d), use_count(%d)\n",
					__func__, rdev->desc->name, rdev->constraints->max_uV, rdev->desc->min_uV,
					rdev->constraints->always_on, rdev->use_count);

		// Make sure enabled registers are cleared
		et5917sx_disable_regmap(rdev);

		ret = et5917sx_recovery_voltage(rdev);
		if (ret < 0)
			return ret;

		if (regulator_should_be_enabled(rdev)) {
			ret = et5917sx_enable(rdev);
			if (ret < 0)
				return ret;
		}
	}

	pr_info("%s: et5917sx is successfully recovered!\n", __func__);

	return ret;
}

static int et5917sx_regulator_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct et5917sx_data *et5917sx = platform_get_drvdata(pdev);

	pr_info("%s: Check recovery needs\n", __func__);

	if (et5917sx_need_self_recovery(et5917sx))
		et5917sx_recovery(et5917sx);

	return 0;
}

const struct dev_pm_ops et5917sx_regulator_pm = {
	.resume = et5917sx_regulator_resume,
};
#endif

static void et5917sx_irq_work(struct work_struct *work)
{
	struct et5917sx_data *et5917sx = container_of(work, struct et5917sx_data, work);
	int i = 0;
	u8 intr1 = 0, intr2 = 0, intr3 = 0;
	u8 intr12_mask[ET5917SX_REGULATOR_MAX] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40};
	u8 intr3_mask[ET5917SX_REGULATOR_MAX] = {0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x10};

	// UVP interrupt
	if (et5917sx_read_reg(et5917sx->iodev->i2c, ET5917SX_REG_UVP_INT, &intr1) < 0)
		goto _out;
	// OCP interrupt
	if (et5917sx_read_reg(et5917sx->iodev->i2c, ET5917SX_REG_OCP_INT, &intr2) < 0)
		goto _out;
	// TSD / UVLO interrupt
	if (et5917sx_read_reg(et5917sx->iodev->i2c, ET5917SX_REG_TSD_UVLO_INT, &intr3) < 0)
		goto _out;

	/* Once the register is read, the interrupt flag bit can be cleared and the
	 * type of event that triggered the interrupt can be obtained, so no interrupt
	 * status register is required.
	 */
	dev_info(et5917sx->iodev->dev, "%s: 0x%02x 0x%02x 0x%02x\n", __func__, intr1, intr2, intr3);

	/*notify..*/
	for (i = 0; i < et5917sx->num_regulators; i++) {
		struct regulator_dev *rdev = et5917sx->rdev[i];

		if (intr1 & intr12_mask[rdev->desc->id] || intr3 & intr3_mask[rdev->desc->id]) {
			regulator_notifier_call_chain(rdev, REGULATOR_EVENT_UNDER_VOLTAGE, NULL);
			dev_info(et5917sx->iodev->dev, "%s: %s REGULATOR_EVENT_UNDER_VOLTAGE\n",
								__func__, rdev->desc->name);
		}

		if (intr2 & intr12_mask[rdev->desc->id]) {
			regulator_notifier_call_chain(rdev, REGULATOR_EVENT_OVER_CURRENT, NULL);
			dev_info(et5917sx->iodev->dev, "%s: %s REGULATOR_EVENT_OVER_CURRENT\n",
								__func__, rdev->desc->name);
		}

		if (intr3 & 0x80 || intr3 & 0x40) {
			regulator_notifier_call_chain(rdev, REGULATOR_EVENT_OVER_TEMP, NULL);
			dev_info(et5917sx->iodev->dev, "%s: %s REGULATOR_EVENT_OVER_TEMP\n",
								__func__, rdev->desc->name);
		}

		if (intr3 & 0x20) {
			regulator_notifier_call_chain(rdev, REGULATOR_EVENT_FAIL, NULL);
			dev_info(et5917sx->iodev->dev, "%s: %s REGULATOR_EVENT_FAIL\n", __func__, rdev->desc->name);
		}
	}

	et5917sx_read_reg(et5917sx->iodev->i2c, ET5917SX_REG_UVP_INT, &intr1);
	et5917sx_read_reg(et5917sx->iodev->i2c, ET5917SX_REG_OCP_INT, &intr2);
	et5917sx_read_reg(et5917sx->iodev->i2c, ET5917SX_REG_TSD_UVLO_INT, &intr3);

_out:
	enable_irq(et5917sx->iodev->et5917sx_irq);

}

static irqreturn_t et5917sx_irq_thread(int irq, void *irq_data)
{
	struct et5917sx_data *et5917sx = (struct et5917sx_data *)irq_data;

	dev_info(et5917sx->iodev->dev, "%s: interrupt occurred(%d)\n", __func__, irq);

	disable_irq_nosync(irq);

	queue_work(et5917sx->wq, &et5917sx->work);

	return IRQ_HANDLED;
}


#if IS_ENABLED(CONFIG_OF)
static int et5917sx_dt_parse_pdata(struct device *dev,
					struct et5917sx_platform_data *pdata)
{
	struct device_node *et5917sx_np, *regulators_np, *reg_np;
	struct et5917sx_regulator_data *rdata;
	size_t i;

	et5917sx_np = dev->of_node;
	if (!et5917sx_np) {
		dev_err(dev, "could not find et5917sx sub-node\n");
		return -ENODEV;
	}

	pdata->et5917sx_irq_gpio = of_get_named_gpio(et5917sx_np, "et5917,et5917_int", 0);
	if (!gpio_is_valid(pdata->et5917sx_irq_gpio)) {
		dev_err(dev, "%s error reading et5917_irq = %d\n", __func__, pdata->et5917sx_irq_gpio);
		pdata->et5917sx_irq_gpio = -ENXIO;
	}

	if (of_property_read_u32(et5917sx_np, "et5917,et5917_int_level", &pdata->et5917sx_int_level) < 0)
		pdata->et5917sx_int_level = 1; // set as register default value

	if (of_property_read_u32(et5917sx_np, "et5917,et5917_int_outmode", &pdata->et5917sx_int_outmode) < 0)
		pdata->et5917sx_int_outmode = 1; // set as register default value

	pdata->wakeup = of_property_read_bool(et5917sx_np, "et5917,wakeup");

	regulators_np = of_find_node_by_name(et5917sx_np, "regulators");
	if (!regulators_np) {
		dev_err(dev, "could not find regulators sub-node\n");
		return -EINVAL;
	}

	pdata->need_self_recovery = of_property_read_bool(et5917sx_np, "et5917sx,need_self_recovery");

	/* count the number of regulators to be supported in et5917 data */
	pdata->num_regulators = 0;
	for_each_child_of_node(regulators_np, reg_np) {
		pdata->num_regulators++;
	}

	rdata = devm_kzalloc(dev, sizeof(*rdata) * pdata->num_regulators, GFP_KERNEL);
	if (!rdata) {
		dev_err(dev, "could not allocate memory for regulator data\n");
		return -ENOMEM;
	}

	pdata->regulators = rdata;
	pdata->num_rdata = 0;
	for_each_child_of_node(regulators_np, reg_np) {
		for (i = 0; i < ARRAY_SIZE(regulators); i++)
			if (!of_node_cmp(reg_np->name, regulators[i].name))
				break;

		if (i == ARRAY_SIZE(regulators)) {
			dev_warn(dev, "don't know how to configure regulator %s\n", reg_np->name);
			continue;
		}

		rdata->id = i;
		rdata->initdata = of_get_regulator_init_data(dev, reg_np, &regulators[i]);
		rdata->reg_node = reg_np;
		rdata++;
		pdata->num_rdata++;
	}
	of_node_put(regulators_np);

	return 0;
}
#else
static int et5917sx_dt_parse_pdata(struct et5917sx_dev *iodev,
					struct et5917sx_platform_data *pdata)
{
	return 0;
}
#endif /* CONFIG_OF */

#if IS_ENABLED(CONFIG_DRV_SAMSUNG) || defined(_SUPPORT_SYSFS_INTERFACE)
static ssize_t et5917sx_read_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct et5917sx_data *et5917sx = dev_get_drvdata(dev);
	int ret;
	u8 val, reg_addr;

	if (buf == NULL) {
		dev_info(dev, "%s: empty buffer\n", __func__);
		return -1;
	}

	ret = kstrtou8(buf, 0, &reg_addr);
	if (ret < 0)
		dev_err(dev, "%s: fail to transform i2c address\n", __func__);

	ret = et5917sx_read_reg(et5917sx->iodev->i2c, reg_addr, &val);
	if (ret < 0)
		dev_err(dev, "%s: fail to read i2c address\n", __func__);

	dev_info(dev, "%s: reg(0x%02x) data(0x%02x)\n", __func__, reg_addr, val);
	et5917sx->read_addr = reg_addr;
	et5917sx->read_val = val;

	return size;
}

static ssize_t et5917sx_read_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct et5917sx_data *et5917sx = dev_get_drvdata(dev);

	return sprintf(buf, "0x%02hhx: 0x%02hhx\n", et5917sx->read_addr,
							et5917sx->read_val);
}

static ssize_t et5917sx_write_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct et5917sx_data *et5917sx = dev_get_drvdata(dev);
	int ret;
	u8 reg = 0, data = 0;

	if (buf == NULL) {
		dev_err(dev, "%s: empty buffer\n", __func__);
		return size;
	}

	ret = sscanf(buf, "0x%02hhx 0x%02hhx", &reg, &data);
	if (ret != 2) {
		dev_err(dev, "%s: input error\n", __func__);
		return size;
	}

	dev_info(dev, "%s: reg(0x%02hhx) data(0x%02hhx)\n", __func__, reg, data);

	ret = et5917sx_write_reg(et5917sx->iodev->i2c, reg, data);
	if (ret < 0)
		dev_err(dev, "%s: fail to write i2c addr/data\n", __func__);

	return size;
}

static ssize_t et5917sx_write_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "echo (register addr.) (data) > et5917sx_write\n");
}

#define ATTR_REGULATOR	(2)
static struct device_attribute et5917_attrs[] = {
	__ATTR(et5917sx_write, 0644, et5917sx_write_show, et5917sx_write_store),
	__ATTR(et5917sx_read, 0644, et5917sx_read_show, et5917sx_read_store),
};

static int et5917sx_create_sysfs(struct et5917sx_data *et5917sx)
{
	struct device *et5917sx_pmic = et5917sx->dev;
	struct device *dev = et5917sx->iodev->dev;
	char device_name[32] = {0,};
	int err = -ENODEV, i = 0;

	et5917sx->read_addr = 0;
	et5917sx->read_val = 0;

	/* Dynamic allocation for device name */
	snprintf(device_name, sizeof(device_name) - 1, "%s@%s",
		dev_driver_string(dev), dev_name(dev));

#if IS_ENABLED(CONFIG_DRV_SAMSUNG)
	et5917sx_pmic = sec_device_create(et5917sx, device_name);
#elif defined(_SUPPORT_SYSFS_INTERFACE)
	pmic_class = class_create(THIS_MODULE, "pmic");
	if (IS_ERR(pmic_class)) {
		dev_err(dev, "Failed to create class(pmic) %ld\n", PTR_ERR(pmic_class));
		return -1;
	}

	et5917sx_pmic = device_create(pmic_class, NULL, atomic_inc_return(&pmic_dev),
			et5917sx, "%s", device_name);
	if (IS_ERR(et5917sx_pmic))
		dev_err(dev, "Failed to create device %s %ld\n", device_name, PTR_ERR(et5917sx_pmic));
	else
		dev_info(dev, "%s : %s : %d\n", __func__, device_name, et5917sx_pmic->devt);
#endif

	et5917sx->dev = et5917sx_pmic;

	/* Create sysfs entries */
	for (i = 0; i < ATTR_REGULATOR; i++) {
		err = device_create_file(et5917sx_pmic, &et5917_attrs[i]);
		if (err)
			goto remove_pmic_device;
	}

	return 0;

remove_pmic_device:
	for (i--; i >= 0; i--)
		device_remove_file(et5917sx_pmic, &et5917_attrs[i]);
#if IS_ENABLED(CONFIG_DRV_SAMSUNG)
	sec_device_destroy(et5917sx_pmic->devt);
#else
	device_destroy(pmic_class, et5917sx_pmic->devt);
#endif
	return -1;
}
#endif

static int __et5917sx_regulator_probe(struct i2c_client *i2c)
{
	struct et5917sx_dev *iodev;
	struct et5917sx_platform_data *pdata = i2c->dev.platform_data;
	struct regulator_config config = { };
	struct et5917sx_data *et5917sx;
	size_t i;
	int ret = 0;
	u8 val = 0;

	dev_info(&i2c->dev, "%s\n", __func__);

	iodev = devm_kzalloc(&i2c->dev, sizeof(struct et5917sx_dev), GFP_KERNEL);
	if (!iodev) {
		dev_err(&i2c->dev, "%s: Failed to alloc mem for et5917sx\n", __func__);
		return -ENOMEM;
	}

	if (i2c->dev.of_node) {
		pdata = devm_kzalloc(&i2c->dev,
			sizeof(struct et5917sx_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_err(&i2c->dev, "Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_pdata;
		}
		ret = et5917sx_dt_parse_pdata(&i2c->dev, pdata);
		if (ret < 0) {
			dev_err(&i2c->dev, "Failed to get device of_node\n");
			goto err_pdata;
		}

		i2c->dev.platform_data = pdata;
	} else
		pdata = i2c->dev.platform_data;

	iodev->dev = &i2c->dev;
	iodev->i2c = i2c;

	if (pdata) {
		iodev->pdata = pdata;
		iodev->wakeup = pdata->wakeup;
	} else {
		ret = -EINVAL;
		goto err_pdata;
	}
	mutex_init(&iodev->i2c_lock);

	et5917sx = devm_kzalloc(&i2c->dev, sizeof(struct et5917sx_data), GFP_KERNEL);
	if (!et5917sx) {
		dev_err(&i2c->dev, "%s: Failed to alloc mem\n", __func__);
		ret = -ENOMEM;
		goto err_et5917sx_data;
	}

	i2c_set_clientdata(i2c, et5917sx);

	et5917sx->iodev = iodev;
	et5917sx->num_regulators = pdata->num_rdata;
	et5917sx->need_self_recovery = pdata->need_self_recovery;

	/* The bit of vsys should be set to 1 before et5917sx
	 * hardware operation
	 */
	et5917sx_read_reg(i2c, ET5917SX_REG_LDO_EN, &val);
	if (!(val & ET5917SX_EN_VSYS_BIT)) {
		dev_err(&i2c->dev, "%s: et5917sx vsys is not set open\n", __func__);
		et5917sx_update_reg(i2c, ET5917SX_REG_LDO_EN,
			ET5917SX_EN_VSYS_BIT, ET5917SX_EN_VSYS_BIT);
	}

	for (i = 0; i < pdata->num_rdata; i++) {
		int id = pdata->regulators[i].id;

		config.dev = &i2c->dev;
		config.init_data = pdata->regulators[i].initdata;
		config.driver_data = et5917sx;
		config.of_node = pdata->regulators[i].reg_node;
		et5917sx->rdev[i] = devm_regulator_register(&i2c->dev,
							&regulators[id], &config);
		if (IS_ERR(et5917sx->rdev[i])) {
			ret = PTR_ERR(et5917sx->rdev[i]);
			dev_err(&i2c->dev, "regulator init failed for %d\n", id);
			et5917sx->rdev[i] = NULL;
			goto err_et5917sx_data;
		}
	}

	et5917sx_set_interrupt(i2c, pdata->et5917sx_int_level, pdata->et5917sx_int_outmode);

	if (pdata->et5917sx_irq_gpio >= 0) {
		gpio_request(pdata->et5917sx_irq_gpio, "et5917sx_irq_gpio");
		gpio_direction_input(pdata->et5917sx_irq_gpio);

		iodev->et5917sx_irq = gpio_to_irq(pdata->et5917sx_irq_gpio);

		if (iodev->et5917sx_irq > 0) {
			et5917sx->wq = create_singlethread_workqueue("et5917sx-wq");
			if (!et5917sx->wq) {
				dev_err(&i2c->dev, "%s: Failed to Request IRQ\n", __func__);
				goto err_et5917sx_data;
			}

			INIT_WORK(&et5917sx->work, et5917sx_irq_work);

			ret = request_threaded_irq(iodev->et5917sx_irq,
					NULL, et5917sx_irq_thread,
					IRQF_TRIGGER_LOW | IRQF_ONESHOT,
					"et5917sx-irq", et5917sx);
			if (ret) {
				dev_err(&i2c->dev, "%s: Failed to Request IRQ\n", __func__);
				goto err_et5917sx_data;
			}

			if (pdata->wakeup) {
				enable_irq_wake(iodev->et5917sx_irq);
				ret = device_init_wakeup(iodev->dev, pdata->wakeup);
				if (ret < 0) {
					dev_err(&i2c->dev, "%s: Fail to device init wakeup fail(%d)\n", __func__, ret);
					goto err_et5917sx_data;
				}
			}
		} else {
			dev_err(&i2c->dev, "%s: Failed gpio_to_irq(%d)\n", __func__, iodev->et5917sx_irq);
			goto err_et5917sx_data;
		}
	} else {
		dev_err(&i2c->dev, "%s: Interrupt pin was not used.\n", __func__);
	}

#if IS_ENABLED(CONFIG_DRV_SAMSUNG) || defined(_SUPPORT_SYSFS_INTERFACE)
	ret = et5917sx_create_sysfs(et5917sx);
	if (ret < 0) {
		dev_err(&i2c->dev, "%s: et5917sx_create_sysfs fail\n", __func__);
		goto err_et5917sx_data;
	}
#endif
	dev_info(&i2c->dev, "%s: Probe complete.\n", __func__);
	return ret;

err_et5917sx_data:
	mutex_destroy(&iodev->i2c_lock);
	if (et5917sx && et5917sx->wq)
		destroy_workqueue(et5917sx->wq);
err_pdata:
	dev_err(&i2c->dev, "%s: Probe failed.\n", __func__);
	return ret;
}

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id et5917sx_i2c_dt_ids[] = {
	{ .compatible = "etek,et5917sx" },
	{ },
};
#endif /* CONFIG_OF */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
static int et5917sx_regulator_probe(struct i2c_client *i2c)
{
	return __et5917sx_regulator_probe(i2c);
}
#else
static int et5917sx_regulator_probe(struct i2c_client *i2c,
				const struct i2c_device_id *dev_id)
{
	return __et5917sx_regulator_probe(i2c);
}
#endif

static void et5917sx_regulator_remove(struct i2c_client *i2c)
{
	struct et5917sx_data *info = i2c_get_clientdata(i2c);
	struct et5917sx_dev *et5917sx = info->iodev;

#if IS_ENABLED(CONFIG_DRV_SAMSUNG) || defined(_SUPPORT_SYSFS_INTERFACE)
	struct device *et5917sx_pmic = info->dev;
	int i = 0;

	/* Remove sysfs entries */
	for (i = 0; i < ATTR_REGULATOR; i++)
		device_remove_file(et5917sx_pmic, &et5917_attrs[i]);

#if IS_ENABLED(CONFIG_DRV_SAMSUNG)
	sec_device_destroy(et5917sx_pmic->devt);
#else
	device_destroy(pmic_class, et5917sx_pmic->devt);
#endif

#endif

	if (et5917sx->pdata && et5917sx->pdata->et5917sx_irq_gpio)
		gpio_free(et5917sx->pdata->et5917sx_irq_gpio);

	if (et5917sx->et5917sx_irq > 0)
		free_irq(et5917sx->et5917sx_irq, NULL);

	if (info->wq)
		destroy_workqueue(info->wq);
}

#if IS_ENABLED(CONFIG_OF)
static const struct i2c_device_id et5917sx_id[] = {
	{"et5917sx-regulator", 0},
	{},
};
#endif

static struct i2c_driver et5917sx_i2c_driver = {
	.driver = {
		.name = "et5917sx-regulator",
		.owner = THIS_MODULE,
#if IS_ENABLED(CONFIG_OF)
		.of_match_table	= et5917sx_i2c_dt_ids,
#endif /* CONFIG_OF */
#if IS_ENABLED(CONFIG_SEC_FACTORY)
		.pm = &et5917sx_regulator_pm,
#endif
		.suppress_bind_attrs = true,
	},
	.probe = et5917sx_regulator_probe,
	.remove = et5917sx_regulator_remove,
	.id_table = et5917sx_id,
};

module_i2c_driver(et5917sx_i2c_driver);

MODULE_DESCRIPTION("ETEK ET5917SX Regulator Driver");
MODULE_LICENSE("GPL");
