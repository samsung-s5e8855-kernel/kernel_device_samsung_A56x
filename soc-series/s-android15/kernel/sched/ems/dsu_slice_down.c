#include <dt-bindings/soc/samsung/ems.h>
#include <soc/samsung/exynos-pmu-if.h>
#include "ems.h"

#define FULL_SLICE_MODE 0x7
#define HALF_SLICE_MODE 0xB

void set_cluster_ppu_powermode(u32 value)
{
	unsigned int val;

	/* pass value to PMU_SPARE7 */
	exynos_pmu_write(0x3bc, (1 << 16) | (value & 0x1ff));
	/* interrupt generation */
	exynos_pmu_update(0x3c30 , (1 << 7), (1 << 7));
	pr_debug("%s: write %x\n", __func__, value);

	while (1) {
		/* apm completed -> PMU_SPARE0 return ((1 << 17) | ppu_op_policy) */
		exynos_pmu_read(0x3bc, &val);
		pr_debug("%s: read %x\n", __func__, val);

		if (val & (1 << 17))
			break;
	}
}

static int dsu_slice_down_emstune_notifier_call(struct notifier_block *nb,
		unsigned long val, void *v)
{
	struct emstune_set *cur_set = (struct emstune_set *)v;

	if (cur_set->dsu_slice_down.enabled) {
		set_cluster_ppu_powermode(HALF_SLICE_MODE);
	} else {
		set_cluster_ppu_powermode(FULL_SLICE_MODE);
	}

	return NOTIFY_OK;
}

static struct notifier_block dsu_slice_down_emstune_notifier = {
	.notifier_call = dsu_slice_down_emstune_notifier_call,
};

void dsu_slice_down_init(void)
{
	emstune_register_notifier(&dsu_slice_down_emstune_notifier);

	pr_info("EMS: Initialize dsu_slice_down\n");
}
