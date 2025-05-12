/*
 * linux/drivers/video/fbdev/exynos/panel/tft_common/tft_common.c
 *
 * TFT_COMMON Dimming Driver
 *
 * Copyright (c) 2016 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/of_gpio.h>
#include <video/mipi_display.h>
#include "ana6710_a56x.h"
#include "ana6710_a56x_panel.h"
#include "ana6710_a56x_ezop.h"


// int ana6710_a56x_maptbl_getidx_ffc(struct maptbl *tbl)
// {
// 	int idx;
// 	u32 dsi_clk;
// 	struct panel_device *panel = (struct panel_device *)tbl->pdata;
// 	struct panel_info *panel_data = &panel->panel_data;

// 	dsi_clk = panel_data->props.dsi_freq;

// 	switch (dsi_clk) {
// 	case 1328000:
// 		idx = ANA6710_A56X_HS_CLK_1328;
// 		break;
// 	case 1362000:
// 		idx = ANA6710_A56X_HS_CLK_1362;
// 		break;
// 	case 1368000:
// 		idx = ANA6710_A56X_HS_CLK_1368;
// 		break;
// 	default:
// 		panel_err("invalid dsi clock: %d\n", dsi_clk);
// 		idx = ANA6710_A56X_HS_CLK_1362;
// 		break;
// 	}
// 	return maptbl_index(tbl, 0, idx, 0);
// }

int ana6710_a56x_maptbl_getidx_ffc(struct maptbl *tbl)
{
	return maptbl_index(tbl, 0, 0, 0);
}

struct pnobj_func ana6710_a56x_function_table[MAX_ANA6710_A56X_FUNCTION] = {
	[ANA6710_A56X_MAPTBL_GETIDX_FFC] = __PNOBJ_FUNC_INITIALIZER(ANA6710_A56X_MAPTBL_GETIDX_FFC, ana6710_a56x_maptbl_getidx_ffc),
};

__visible_for_testing int __init ana6710_a56x_panel_init(void)
{
	struct common_panel_info *cpi = &ana6710_a56x_panel_info;
	int ret;

	ana6710_init(cpi);
	cpi->ezop_json = EZOP_JSON_BUFFER;

	ret = panel_function_insert_array(ana6710_a56x_function_table,
			ARRAY_SIZE(ana6710_a56x_function_table));
	if (ret < 0)
		panel_err("failed to insert ana6710_a56x_function_table\n");


	register_common_panel(cpi);

	panel_vote_up_to_probe(NULL);

	return 0;
}

__visible_for_testing void __exit ana6710_a56x_panel_exit(void)
{
	deregister_common_panel(&ana6710_a56x_panel_info);
}

module_init(ana6710_a56x_panel_init)
module_exit(ana6710_a56x_panel_exit)

MODULE_DESCRIPTION("Samsung Mobile Panel Driver");
MODULE_LICENSE("GPL");
