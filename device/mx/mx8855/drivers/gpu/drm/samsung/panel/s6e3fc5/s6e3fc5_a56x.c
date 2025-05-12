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
#include "../panel_debug.h"
#include "s6e3fc5_a56x.h"
#include "s6e3fc5_a56x_panel.h"
#include "s6e3fc5_a56x_ezop.h"

int s6e3fc5_a56x_maptbl_getidx_hbm_transition(struct maptbl *tbl)
{
	int layer, row, col;
	struct panel_bl_device *panel_bl;
	struct panel_device *panel = (struct panel_device *)tbl->pdata;

	if (panel == NULL) {
		panel_err("panel is null\n");
		return -EINVAL;
	}

	panel_bl = &panel->panel_bl;

	layer = is_hbm_brightness(panel_bl, panel_bl->props.brightness);
	row = panel_bl->props.smooth_transition;
	col = panel_bl->props.local_hbm;

	panel_dbg("brightness %d, layer %d, row %d, col %d\n", panel_bl->props.brightness, layer, row, col);

	return maptbl_4d_index(tbl, layer, row, col, 0);
}

void s6e3fc5_a56x_maptbl_copy_local_hbm(struct maptbl *tbl, u8 *dst)
{
	struct panel_device *panel;
	struct panel_info *panel_data;
	u8 comp[S6E3FC5_LOCAL_HBM_COMP_LEN] = { 0, };
	int ret;

	if (!tbl) {
		panel_err("tbl is null\n");
		return;
	}

	if (!dst) {
		panel_err("dst is null\n");
		return;
	}

	panel = (struct panel_device *)tbl->pdata;
	if (unlikely(!panel))
		return;

	panel_data = &panel->panel_data;

	ret = panel_resource_copy(panel, comp, "local_hbm_comp");
	if (ret < 0) {
		panel_err("failed to copy local hbm comp resource %d\n", ret);
		return;
	}
	memcpy(dst, comp, sizeof(comp));
}

static struct panel_prop_enum_item s6e3fc5_a56x_hs_clk_enum_items[MAX_S6E3FC5_A56X_HS_CLK] = {
	__PANEL_PROPERTY_ENUM_ITEM_INITIALIZER(S6E3FC5_A56X_HS_CLK_1108),
	__PANEL_PROPERTY_ENUM_ITEM_INITIALIZER(S6E3FC5_A56X_HS_CLK_1124),
	__PANEL_PROPERTY_ENUM_ITEM_INITIALIZER(S6E3FC5_A56X_HS_CLK_1125),
};

static int s6e3fc5_a56x_hs_clk_property_update(struct panel_property *prop)
{
	struct panel_device *panel = prop->panel;
	u32 dsi_clk = panel_get_property_value(panel, PANEL_PROPERTY_DSI_FREQ);
	int index;

	switch (dsi_clk) {
		case 1108000:
			index = S6E3FC5_A56X_HS_CLK_1108;
			break;
		case 1124000:
			index = S6E3FC5_A56X_HS_CLK_1124;
			break;
		case 1125000:
			index = S6E3FC5_A56X_HS_CLK_1125;
			break;
		default:
			panel_err("invalid dsi clock: %d, use default clk 1108000\n", dsi_clk);
			index = S6E3FC5_A56X_HS_CLK_1108;
			break;
	}
	panel_dbg("dsi clock: %d, index: %d\n", dsi_clk, index);

	return panel_property_set_value(prop, index);
}

struct pnobj_func s6e3fc5_a56x_function_table[MAX_S6E3FC5_A56X_FUNCTION] = {
	[S6E3FC5_A56X_MAPTBL_GETIDX_HBM_TRANSITION] = __PNOBJ_FUNC_INITIALIZER(S6E3FC5_A56X_MAPTBL_GETIDX_HBM_TRANSITION, s6e3fc5_a56x_maptbl_getidx_hbm_transition),
	[S6E3FC5_A56X_MAPTBL_COPY_LOCAL_HBM_COMP] = __PNOBJ_FUNC_INITIALIZER(S6E3FC5_A56X_MAPTBL_COPY_LOCAL_HBM_COMP, s6e3fc5_a56x_maptbl_copy_local_hbm),
};

static struct panel_prop_enum_item s6e3fc5_a56x_osc_clk_enum_items[MAX_S6E3FC5_A56X_OSC_CLK] = {
	__PANEL_PROPERTY_ENUM_ITEM_INITIALIZER(S6E3FC5_A56X_OSC_CLK_181300),
	__PANEL_PROPERTY_ENUM_ITEM_INITIALIZER(S6E3FC5_A56X_OSC_CLK_178900),
};

int s6e3fc5_a56x_osc_update(struct panel_device *panel)
{
	u32 osc_clk, old_idx, new_idx;

	if (panel == NULL) {
		panel_err("panel is null\n");
		return -EINVAL;
	}

	osc_clk = panel_get_property_value(panel, PANEL_PROPERTY_OSC_FREQ);

	switch (osc_clk) {
		case 181300:
			new_idx = S6E3FC5_A56X_OSC_CLK_181300;
			break;
		case 178900:
			new_idx = S6E3FC5_A56X_OSC_CLK_178900;
			break;
		default:
			panel_err("invalid osc clock: %d, use default clk 181300\n", osc_clk);
			new_idx = S6E3FC5_A56X_OSC_CLK_181300;
			break;
	}
	panel_dbg("osc clock: %d, index: %d\n", osc_clk, new_idx);

	old_idx = panel_get_property_value(panel, S6E3FC5_A56X_OSC_CLK_PROPERTY);
	if (old_idx != new_idx) {
		panel_info("osc clock idx updated: %d -> %d\n", old_idx, new_idx);
		panel_set_property_value(panel, S6E3FC5_A56X_OSC_CLK_PROPERTY, new_idx);
	}

	return 0;
}

static struct panel_prop_list s6e3fc5_a56x_property_array[] = {
	__DIMEN_PROPERTY_ENUM_INITIALIZER(S6E3FC5_A56X_HS_CLK_PROPERTY,
			S6E3FC5_A56X_HS_CLK_1108, s6e3fc5_a56x_hs_clk_enum_items,
			s6e3fc5_a56x_hs_clk_property_update),
	__DIMEN_PROPERTY_ENUM_INITIALIZER(S6E3FC5_A56X_OSC_CLK_PROPERTY,
			S6E3FC5_A56X_OSC_CLK_181300, s6e3fc5_a56x_osc_clk_enum_items,
			NULL),
};

int s6e3fc5_a56x_ddi_init(struct panel_device *panel, void *buf, u32 len)
{
	return s6e3fc5_a56x_osc_update(panel);
}

__visible_for_testing int __init s6e3fc5_a56x_panel_init(void)
{
	struct common_panel_info *cpi = &s6e3fc5_a56x_panel_info;
	int ret;

	s6e3fc5_init(cpi);
	cpi->prop_lists[USDM_DRV_LEVEL_MODEL] = s6e3fc5_a56x_property_array;
	cpi->num_prop_lists[USDM_DRV_LEVEL_MODEL] = ARRAY_SIZE(s6e3fc5_a56x_property_array);
	cpi->ezop_json = EZOP_JSON_BUFFER;
	register_common_panel(cpi);

	ret = panel_function_insert_array(s6e3fc5_a56x_function_table,
			ARRAY_SIZE(s6e3fc5_a56x_function_table));
	if (ret < 0)
		panel_err("failed to insert s6e3fc5_a56x_function_table\n");

	panel_vote_up_to_probe(NULL);

	return 0;
}

__visible_for_testing void __exit s6e3fc5_a56x_panel_exit(void)
{
	deregister_common_panel(&s6e3fc5_a56x_panel_info);
}

module_init(s6e3fc5_a56x_panel_init)
module_exit(s6e3fc5_a56x_panel_exit)

MODULE_DESCRIPTION("Samsung Mobile Panel Driver");
MODULE_LICENSE("GPL");
