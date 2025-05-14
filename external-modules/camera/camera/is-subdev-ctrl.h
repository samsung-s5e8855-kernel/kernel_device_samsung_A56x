// SPDX-License-Identifier: GPL-2.0
/*
 * Samsung Exynos SoC series Pablo driver
 *
 * Copyright (c) 2021 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef IS_SUBDEV_H
#define IS_SUBDEV_H

#include "is-video.h"
#include "pablo-work.h"

#define SUBDEV_INTERNAL_BUF_MAX		(8)

struct is_device_sensor;
struct is_device_ischain;
struct is_subdev;
struct is_frame;

enum is_subdev_device_type {
	IS_SENSOR_SUBDEV,
	IS_ISCHAIN_SUBDEV,
};

enum is_subdev_state {
	IS_SUBDEV_OPEN,
	IS_SUBDEV_START,
	IS_SUBDEV_RUN,
	IS_SUBDEV_FORCE_SET,
	IS_SUBDEV_EXTERNAL_USE,
};

enum pablo_subdev_get_type {
	PSGT_REGION_NUM,
};

struct is_subdev_path {
	u32		width;
	u32		height;
	struct is_crop	canv;
	struct is_crop	crop;
};

/* Caution: Do not exceed 64 */
enum is_subdev_id {
	ENTRY_SENSOR,
	ENTRY_SSVC0,
	ENTRY_SSVC1,
	ENTRY_SSVC2,
	ENTRY_SSVC3,
	ENTRY_SSVC4,
	ENTRY_SSVC5,
	ENTRY_SSVC6,
	ENTRY_SSVC7,
	ENTRY_SSVC8,
	ENTRY_SSVC9,
	ENTRY_3AA,
	ENTRY_ISP,
	ENTRY_MCS,
	ENTRY_M0P,
	ENTRY_M1P,
	ENTRY_M2P,
	ENTRY_M3P,
	ENTRY_M4P,
	ENTRY_M5P,
	ENTRY_PAF, /* PDP(PATSTAT) Bayer RDMA */
	ENTRY_PDAF, /* PDP(PATSTAT) AF RDMA */
	ENTRY_ORB, /* ORBMCH */
	ENTRY_YUVP, /* YUVP */
	ENTRY_SHRP, /* SHRP */
	ENTRY_SHRP_HF, /* SHRP */
	ENTRY_LME,
	ENTRY_LMES,
	ENTRY_LMEC,

	ENTRY_BYRP,
	ENTRY_RGBP,
	ENTRY_RGBP_HF,
	ENTRY_MCFP,

	ENTRY_YUVSC,
	ENTRY_MLSC,
	ENTRY_MTNR,
	ENTRY_MSNR,

	ENTRY_END,
};

#define ENTRY_MCSC_P5		ENTRY_M5P

struct is_subdev_ops {
	int (*bypass)(struct is_subdev *subdev,
		void *device_data,
		struct is_frame *frame,
		bool bypass);
	int (*cfg)(struct is_subdev *subdev,
		void *device_data,
		struct is_frame *frame,
		struct is_crop *incrop,
		struct is_crop *otcrop,
		IS_DECLARE_PMAP(pmap));
	int (*tag)(struct is_subdev *subdev,
		void *device_data,
		struct is_frame *frame,
		struct camera2_node *node);
	int (*get)(struct is_subdev *subdev,
		   struct is_frame *frame,
		   enum pablo_subdev_get_type type,
		   void *result);
};

struct is_subdev {
	u32				id;
	u32				vid; /* video id */
	u32				cid; /* capture node id */
	const char			*name;
	char				*stm;
	u32				instance;
	unsigned long			state;

	u32				constraints_width; /* spec in width */
	u32				constraints_height; /* spec in height */

	u32				param_otf_in;
	u32				param_dma_in;
	u32				param_otf_ot;
	u32				param_dma_ot;

	struct is_subdev_path		input;
	struct is_subdev_path		output;

	struct list_head		list;

	u32				bits_per_pixel;
	u32				memory_bitwidth;
	u32				sbwc_type;

	struct is_video_ctx		*vctx;
	struct is_subdev		*leader;
	const struct is_subdev_ops	*ops;
};

int is_sensor_subdev_open(struct is_device_sensor *device,
	struct is_video_ctx *vctx);
int is_sensor_subdev_close(struct is_device_sensor *device,
	struct is_video_ctx *vctx);

int is_ischain_subdev_open(struct is_device_ischain *device,
	struct is_video_ctx *vctx);
int is_ischain_subdev_close(struct is_device_ischain *device,
	struct is_video_ctx *vctx);

/*common subdev*/
int is_subdev_probe(struct is_subdev *subdev,
	u32 instance,
	u32 id,
	const char *name,
	const struct is_subdev_ops *sops);
int is_subdev_open(struct is_subdev *subdev, struct is_video_ctx *vctx, u32 instance);
int is_subdev_close(struct is_subdev *subdev);

struct is_queue_ops *is_get_sensor_subdev_qops(void);
struct is_queue_ops *is_get_ischain_subdev_qops(void);

int __mcsc_dma_out_cfg(struct is_device_ischain *device,
	struct is_frame *ldr_frame,
	struct camera2_node *node,
	u32 pindex,
	IS_DECLARE_PMAP(pmap),
	int index);

#define GET_SUBDEV_FRAMEMGR(subdev)                                                                \
	({                                                                                         \
		struct is_framemgr *framemgr;                                                      \
		if ((subdev) && (subdev)->vctx && (subdev)->vctx->queue)                           \
			framemgr = &(subdev)->vctx->queue->framemgr;                               \
		else                                                                               \
			framemgr = NULL;                                                           \
		framemgr;                                                                          \
	})

#define GET_SUBDEV_QUEUE(subdev) \
	(((subdev) && (subdev)->vctx) ? ((subdev)->vctx->queue) : NULL)
#define CALL_SOPS(s, op, args...)	(((s) && (s)->ops && (s)->ops->op) ? ((s)->ops->op(s, args)) : 0)

#endif
