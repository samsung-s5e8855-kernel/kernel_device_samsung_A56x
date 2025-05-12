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

#ifndef __ANA6710_A56X_H__
#define __ANA6710_A56X_H__
#include "../panel.h"
#include "../panel_drv.h"
#include "../panel_debug.h"

enum {
	ANA6710_A56X_HS_CLK_1328 = 0,
	ANA6710_A56X_HS_CLK_1362,
	ANA6710_A56X_HS_CLK_1368,
	MAX_ANA6710_A56X_HS_CLK
};

enum ana6710_a56x_function {
	ANA6710_A56X_MAPTBL_GETIDX_FFC,
	MAX_ANA6710_A56X_FUNCTION,
};

extern struct pnobj_func ana6710_a56x_function_table[MAX_ANA6710_A56X_FUNCTION];

#undef DDI_PROJECT_FUNC
#define DDI_PROJECT_FUNC(_index) (ana6710_a56x_function_table[_index])

#endif /* __ANA6710_A56X_H__ */
