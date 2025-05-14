/*
 * linux/drivers/gpu/drm/samsung/panel/ana6710/ana6710_a56x_rcd.h
 *
 * Header file for Panel Driver
 *
 * Copyright (c) 2022 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ANA6710_A56X_RCD_H__
#define __ANA6710_A56X_RCD_H__

#include "../panel.h"
#include "ana6710.h"
#include "ana6710_a56x_rcd_image.h"

static struct rcd_image ana6710_a56x_rcd_1080x2340_top = {
	.name = "top",
	.image_rect = {
		.x = 0,
		.y = 0,
		.w = 1080,
		.h = 144,
	},
	.image_data = ANA6710_A56X_RCD_IMAGE_TOP,
	.image_data_len = ARRAY_SIZE(ANA6710_A56X_RCD_IMAGE_TOP),
};

static struct rcd_image ana6710_a56x_rcd_1080x2340_bottom = {
	.name = "bottom",
	.image_rect = {
		.x = 0,
		.y = 2196,
		.w = 1080,
		.h = 144,
	},
	.image_data = ANA6710_A56X_RCD_IMAGE_BOTTOM,
	.image_data_len = ARRAY_SIZE(ANA6710_A56X_RCD_IMAGE_BOTTOM),
};

static struct rcd_image *ana6710_a56x_rcd_1080x2340_images[] = {
	&ana6710_a56x_rcd_1080x2340_top,
	&ana6710_a56x_rcd_1080x2340_bottom,
};

static struct rcd_resol ana6710_a56x_rcd_resol[] = {
	[ANA6710_RESOL_1080x2340] = {
		.name = "ana6710_a56x_rcd_1080x2340",
		.resol_x = 1080,
		.resol_y = 2340,
		.block_rect = {
			.x = 0,
			.y = 144,
			.w = 1080,
			.h = 2052,
		},
		.images = ana6710_a56x_rcd_1080x2340_images,
		.nr_images = ARRAY_SIZE(ana6710_a56x_rcd_1080x2340_images),
	},
};

static struct rcd_resol *ana6710_a56x_rcd_resol_array[] = {
	&ana6710_a56x_rcd_resol[ANA6710_RESOL_1080x2340],
};

static struct panel_rcd_data ana6710_a56x_rcd = {
	.version = 1,
	.name = "ana6710_a56x_rcd",
	.rcd_resol = ana6710_a56x_rcd_resol_array,
	.nr_rcd_resol = ARRAY_SIZE(ana6710_a56x_rcd_resol_array),
};

#endif
