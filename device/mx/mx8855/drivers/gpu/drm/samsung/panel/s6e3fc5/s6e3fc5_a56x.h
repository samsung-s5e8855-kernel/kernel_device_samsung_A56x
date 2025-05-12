/*
 * linux/drivers/video/fbdev/exynos/panel/nt36672c_m33x_00/nt36672c_m33_00.h
 *
 * Header file for TFT_COMMON Dimming Driver
 *
 * Copyright (c) 2016 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __S6E3FC5_A56X_H__
#define __S6E3FC5_A56X_H__
#include "../panel.h"
#include "../panel_drv.h"

enum s6e3fc5_a56x_function {
    S6E3FC5_A56X_MAPTBL_GETIDX_HBM_TRANSITION,
    S6E3FC5_A56X_MAPTBL_COPY_LOCAL_HBM_COMP,
	MAX_S6E3FC5_A56X_FUNCTION,
};

enum {
	S6E3FC5_A56X_HS_CLK_1108 = 0,
	S6E3FC5_A56X_HS_CLK_1124,
	S6E3FC5_A56X_HS_CLK_1125,
	MAX_S6E3FC5_A56X_HS_CLK
};

enum {
	S6E3FC5_A56X_OSC_CLK_181300 = 0,
	S6E3FC5_A56X_OSC_CLK_178900,
	MAX_S6E3FC5_A56X_OSC_CLK
};

#define S6E3FC5_A56X_HS_CLK_PROPERTY ("s6e3fc5_a56x_hs_clk")
#define S6E3FC5_A56X_OSC_CLK_PROPERTY ("s6e3fc5_a56x_osc_clk")

extern struct pnobj_func s6e3fc5_a56x_function_table[MAX_S6E3FC5_A56X_FUNCTION];

#undef PANEL_FUNC
#define PANEL_FUNC(_index) (s6e3fc5_a56x_function_table[_index])

int s6e3fc5_a56x_ddi_init(struct panel_device *panel, void *buf, u32 len);

#endif /* __S6E3FC5_A56X_H__ */
