/* SPDX-License-Identifier: GPL-2.0-only
 * exynos_drm_crtc.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Authors:
 *	Inki Dae <inki.dae@samsung.com>
 *	Joonyoung Shim <jy0922.shim@samsung.com>
 *	Seung-Woo Kim <sw0312.kim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef _EXYNOS_DRM_CRTC_H_
#define _EXYNOS_DRM_CRTC_H_

#include <exynos_drm_drv.h>
#include <exynos_drm_debug.h>
#include <exynos_drm_connector.h>
#include <drm/drm_encoder.h>

/* HAL color mode */
enum HAL_color_mode {
	HAL_COLOR_MODE_NATIVE = 0,
	HAL_COLOR_MODE_STANDARD_BT601_625 = 1,
	HAL_COLOR_MODE_STANDARD_BT601_625_UNADJUSTED = 2,
	HAL_COLOR_MODE_STANDARD_BT601_525 = 3,
	HAL_COLOR_MODE_STANDARD_BT601_525_UNADJUSTED = 4,
	HAL_COLOR_MODE_STANDARD_BT709 = 5,
	HAL_COLOR_MODE_DCI_P3 = 6,
	HAL_COLOR_MODE_SRGB = 7,
	HAL_COLOR_MODE_ADOBE_RGB = 8,
	HAL_COLOR_MODE_DISPLAY_P3 = 9,
	HAL_COLOR_MODE_BT2020 = 10,
	HAL_COLOR_MODE_BT2100_PQ = 11,
	HAL_COLOR_MODE_BT2100_HLG = 12,
	HAL_COLOR_MODE_NUM_MAX,
	HAL_COLOR_MODE_CUSTOM_0 = 0x100,
	HAL_COLOR_MODE_CUSTOM_1,
	HAL_COLOR_MODE_CUSTOM_2,
	HAL_COLOR_MODE_CUSTOM_3,
	HAL_COLOR_MODE_CUSTOM_4,
	HAL_COLOR_MODE_CUSTOM_5,
	HAL_COLOR_MODE_CUSTOM_6,
	HAL_COLOR_MODE_CUSTOM_7,
	HAL_COLOR_MODE_CUSTOM_8,
	HAL_COLOR_MODE_CUSTOM_9,
	HAL_COLOR_MODE_CUSTOM_10,
	HAL_COLOR_MODE_CUSTOM_11,
	HAL_COLOR_MODE_CUSTOM_12,
	HAL_COLOR_MODE_CUSTOM_13,
	HAL_COLOR_MODE_CUSTOM_14,
	HAL_COLOR_MODE_CUSTOM_15,
	HAL_COLOR_MODE_CUSTOM_16,
	HAL_COLOR_MODE_CUSTOM_17,
	HAL_COLOR_MODE_CUSTOM_18,
	HAL_COLOR_MODE_CUSTOM_19,
	HAL_COLOR_MODE_CUSTOM_20,
	HAL_COLOR_MODE_CUSTOM_21,
	HAL_COLOR_MODE_CUSTOM_22,
	HAL_COLOR_MODE_CUSTOM_23,
	HAL_COLOR_MODE_CUSTOM_24,
	HAL_COLOR_MODE_CUSTOM_25,
	HAL_COLOR_MODE_CUSTOM_26,
	HAL_COLOR_MODE_CUSTOM_27,
	HAL_COLOR_MODE_CUSTOM_28,
	HAL_COLOR_MODE_CUSTOM_29,
};

/* HAL intent info */
enum HAL_intent_info {
	HAL_RENDER_INTENT_COLORIMETRIC = 0,
	HAL_RENDER_INTENT_ENHANCE = 1,
	HAL_RENDER_INTENT_TONE_MAP_COLORIMETRIC = 2,
	HAL_RENDER_INTENT_TONE_MAP_ENHANCE = 3,
	HAL_RENDER_INTENT_CUSTOM_0 = 0x100,
	HAL_RENDER_INTENT_CUSTOM_1,
	HAL_RENDER_INTENT_CUSTOM_2,
	HAL_RENDER_INTENT_CUSTOM_3,
	HAL_RENDER_INTENT_CUSTOM_4,
	HAL_RENDER_INTENT_CUSTOM_5 = 0x10a,
	HAL_RENDER_INTENT_CUSTOM_6,
	HAL_RENDER_INTENT_CUSTOM_7,
	HAL_RENDER_INTENT_CUSTOM_8,
	HAL_RENDER_INTENT_CUSTOM_9,
	HAL_RENDER_INTENT_CUSTOM_10 = 0x114,
	HAL_RENDER_INTENT_CUSTOM_11,
	HAL_RENDER_INTENT_CUSTOM_12,
	HAL_RENDER_INTENT_CUSTOM_13,
	HAL_RENDER_INTENT_CUSTOM_14,
	HAL_RENDER_INTENT_CUSTOM_15 = 0x11e,
	HAL_RENDER_INTENT_CUSTOM_16,
	HAL_RENDER_INTENT_CUSTOM_17,
	HAL_RENDER_INTENT_CUSTOM_18,
	HAL_RENDER_INTENT_CUSTOM_19,
	HAL_RENDER_INTENT_CUSTOM_20 = 0x128,
	HAL_RENDER_INTENT_CUSTOM_21,
	HAL_RENDER_INTENT_CUSTOM_22,
	HAL_RENDER_INTENT_CUSTOM_23,
	HAL_RENDER_INTENT_CUSTOM_24,
};

/* HAL color transform*/
enum HAL_color_transform {
	HAL_COLOR_TRANSFORM_IDENTITY = 0,
	HAL_COLOR_TRANSFORM_ARBITRARY_MATRIX = 1,
	HAL_COLOR_TRANSFORM_VALUE_INVERSE = 2,
	HAL_COLOR_TRANSFORM_GRAYSCALE = 3,
	HAL_COLOR_TRANSFORM_CORRECT_PROTANOPIA = 4,
	HAL_COLOR_TRANSFORM_CORRECT_DEUTERANOPIA = 5,
	HAL_COLOR_TRANSFORM_CORRECT_TRITANOPIA = 6,
};

#define to_exynos_crtc(x)	container_of(x, struct exynos_drm_crtc, base)

enum exynos_drm_op_mode {
	EXYNOS_VIDEO_MODE,
	EXYNOS_COMMAND_MODE,
};

enum exynos_drm_crc_state {
	EXYNOS_DRM_CRC_STOP,
	EXYNOS_DRM_CRC_REQ,
	EXYNOS_DRM_CRC_START,
	EXYNOS_DRM_CRC_PEND,
};

enum exynos_drm_scaler_type {
	EXYNOS_SCALER_NONE = 0, /* None or DDI scaler : Default */
	EXYNOS_SCALER_DQE,
	EXYNOS_SCALER_AIQE,
	EXYNOS_SCALER_MAX,
};

static inline bool IS_PSCALER_ENABLED(enum exynos_drm_scaler_type type)
{
	return (type == EXYNOS_SCALER_DQE || type == EXYNOS_SCALER_AIQE);
}

/*
 * Exynos drm crtc configuration structure.
 *
 * @default_plane: a pointer to default plane of the crtc.
 * @con_type: output type of crtc to determine the possible encoder.
 * @res: decon restriction.
 * @ctx: a pointer to the decon device.
 *
 * The configuration structure used to create exynos crtc structure.
 */

struct exynos_drm_crtc_config {
	struct drm_plane *default_plane;
	enum exynos_drm_output_type con_type;
	const struct decon_restrict *res;
	void *ctx;
};

/*
 * Exynos drm crtc ops
 *
 * @enable: enable the device
 * @disable: disable the device
 * @atomic_enter_hiber: enter the device to hibernation with atomic commit.
 * @atomic_exit_hiber: exit the device from hibernation with atomic commit.
 * @enable_vblank: specific driver callback for enabling vblank interrupt.
 * @disable_vblank: specific driver callback for disabling vblank interrupt.
 * @mode_valid: specific driver callback for mode validation
 * @atomic_check: validate state
 * @atomic_begin: prepare device to receive an update
 * @atomic_flush: mark the end of device update
 * @update_plane: apply hardware specific overlay data to registers.
 * @disable_plane: disable hardware specific overlay.
 * @te_handler: trigger to transfer video image at the tearing effect
 *	synchronization signal if there is a page flip request.
 * @atomic_print_state: printing additional driver specific data.
 * @late_register: register debugfs for hardware specific data.
 * @wait_framestart: wait framestart irq after atomic_flush.
 * @get_trigger_mask: get ref count for decon trigger_mask.
 * @put_trigger_mask: put ref count for decon trigger_mask.
 * @dump_register: dump the registers included in this display pipeline.
 */
struct exynos_drm_crtc;
struct exynos_drm_crtc_state;
struct exynos_drm_plane;
struct dpu_bts;
struct exynos_drm_crtc_ops {
	void (*enable)(struct exynos_drm_crtc *crtc,
		       struct drm_atomic_state *state);
	void (*disable)(struct exynos_drm_crtc *crtc);
	void (*atomic_enter_hiber)(struct exynos_drm_crtc *crtc);
	void (*atomic_exit_hiber)(struct exynos_drm_crtc *crtc);
	int (*enable_vblank)(struct exynos_drm_crtc *crtc);
	void (*disable_vblank)(struct exynos_drm_crtc *crtc);
	u32 (*get_vblank_counter)(struct exynos_drm_crtc *crtc);
	enum drm_mode_status (*mode_valid)(struct exynos_drm_crtc *crtc,
		const struct drm_display_mode *mode);
	bool (*mode_fixup)(struct exynos_drm_crtc *crtc,
		const struct drm_display_mode *mode,
		struct drm_display_mode *adjusted_mode);
	void (*mode_set)(struct exynos_drm_crtc *crtc,
		const struct drm_display_mode *mode,
		const struct drm_display_mode *adjusted_mode);
	int (*atomic_check)(struct exynos_drm_crtc *crtc,
			    struct drm_atomic_state *state);
	void (*atomic_begin)(struct exynos_drm_crtc *crtc,
			     struct drm_atomic_state *state);
	void (*update_plane)(struct exynos_drm_crtc *crtc,
			     struct exynos_drm_plane *plane,
			     struct drm_atomic_state *old_state);
	void (*disable_plane)(struct exynos_drm_crtc *crtc,
			      struct exynos_drm_plane *plane);
	void (*atomic_flush)(struct exynos_drm_crtc *crtc,
			     struct drm_atomic_state *state);
	void (*te_handler)(struct exynos_drm_crtc *crtc);
	void (*atomic_print_state)(struct drm_printer *p,
			const struct exynos_drm_crtc *crtc);
	int (*late_register)(struct exynos_drm_crtc *crtc);
	void (*wait_framestart)(struct exynos_drm_crtc *crtc);
	void (*get_trigger_mask)(struct exynos_drm_crtc *crtc);
	void (*put_trigger_mask)(struct exynos_drm_crtc *crtc);
	void (*update_request)(struct exynos_drm_crtc *crtc);
	bool (*check_svsync_start)(struct exynos_drm_crtc *crtc);
	void (*dump_register)(struct exynos_drm_crtc *crtc);
	void (*pll_sleep_mask)(struct exynos_drm_crtc *crtc, bool mask);
	int (*recovery)(struct exynos_drm_crtc *crtc, char *mode);
	bool (*is_recovering)(struct exynos_drm_crtc *crtc);
	void (*update_bts_fps)(struct exynos_drm_crtc *crtc);
	void (*emergency_off)(struct exynos_drm_crtc *crtc);
	void (*get_crc_data)(struct exynos_drm_crtc *crtc);
#if IS_ENABLED(CONFIG_DRM_MCD_COMMON)
	void (*dump_event_log)(struct exynos_drm_crtc *crtc);
#endif
};

/*
 * Exynos specific crtc state structure.
 *
 * @base: drm crtc state.
 * @old_state: the old global state object for non-blocking commit tail work.
 * @commit_work: work item which can be used by the driver or helpers
 * to execute the non-blocking commit.
 */
struct pscaler_mode {
	enum exynos_drm_scaler_type type;
	u32 hdisplay;
	u32 vdisplay;
};

struct exynos_drm_crtc_state {
	struct drm_crtc_state base;
	struct drm_atomic_state *old_state;
	struct kthread_work commit_work;
	uint32_t color_mode;
	uint32_t render_intent;
	uint32_t in_bpc;
	bool dsr_status : 1;
	bool tui_status : 1;
	bool tui_changed : 1;
	bool modeset_post_process : 1;
	bool skip_frameupdate : 1;
	bool skip_hw_update : 1;
	bool need_colormap : 1;
	bool initial_commit_completed : 1;
	bool shutdown_inprogress : 1;
	struct drm_rect partial_region;
	struct drm_property_blob *partial;
	bool needs_reconfigure;
	enum exynos_drm_writeback_type wb_type;
	unsigned int reserved_win_mask;
	unsigned int visible_win_mask;
	unsigned int freed_win_mask;
	int64_t color_fd_slot0;
	int64_t color_fd_slot1;
	s32 __user *bts_fps_ptr;
	u32 boost_bts_fps;
	bool rcd_enabled;
	u64 expected_present_time_ns;
	u64 adjusted_present_time_ns;
	u32 frame_interval_ns;
	char *dqe_colormode_ctx;
	struct pscaler_mode pscaler;
	struct exynos_display_mode exynos_mode;
	unsigned int seamless_modeset;
#if IS_ENABLED(CONFIG_DRM_MCD_COMMON)
	bool skip_frame_update;
#endif
};

static inline struct exynos_drm_crtc_state *
to_exynos_crtc_state(const struct drm_crtc_state *state)
{
	return container_of(state, struct exynos_drm_crtc_state, base);
}

/*
 * Exynos specific crtc structure.
 *
 * @base: crtc object.
 * @dev: A back pointer of device.
 * @possible_type: what can connect connector types
 * @ops: pointer to callbacks for exynos drm specific functionality
 * @ctx: A pointer to the crtc's implementation specific context
 * @pipe_clk: A pointer to the crtc's pipeline clock.
 * @dpu_bts: A back pointer of bts to refer bts in common layer.
 * @event: drm pending event is protected by dev->event_lock.
 */
struct exynos_dqe;
struct exynos_drm_crtc {
	struct drm_crtc			base;
	struct device			*dev;
	enum exynos_drm_output_type	possible_type;
	const struct exynos_drm_crtc_ops	*ops;
	struct dpu_debug		d;
	struct exynos_dqe		*dqe;
	void				*vmc;
	void				*self_refresh;
	void				*ctx;
	struct exynos_hibernation	*hibernation;
	struct task_struct		*thread;
	struct kthread_worker		worker;
	void		 		*profiler;
	struct exynos_partial 		*partial;
	struct dpu_bts			*bts;
	struct dpu_freq_hop_ops		*freq_hop;
	enum exynos_drm_crc_state	crc_state;
	struct drm_pending_vblank_event *event;
	u32 rcd_plane_mask;
};

int exynos_drm_crtc_create_properties(struct drm_device *drm_dev);
struct exynos_drm_crtc *
exynos_drm_crtc_create(struct drm_device *drm_dev, unsigned int index,
		       struct exynos_drm_crtc_config *config,
		       const struct exynos_drm_crtc_ops *ops);

/* This function gets crtc device matched with out_type. */
struct exynos_drm_crtc *exynos_drm_crtc_get_by_type(struct drm_device *drm_dev,
				       enum exynos_drm_output_type out_type);

uint32_t exynos_drm_get_possible_crtcs(struct drm_encoder *encoder,
		enum exynos_drm_output_type out_type);

void exynos_crtc_arm_event(struct exynos_drm_crtc *exynos_crtc);
void exynos_crtc_send_event(struct exynos_drm_crtc *exynos_crtc);
void exynos_drm_crtc_handle_vblank(struct exynos_drm_crtc *exynos_crtc);

/*
 * Compute whether the colormap is needed. If there is no plane and
 * color_mgmt_changed, and crtc use the LCD or DP path, the colormap is needed.
 */
static inline bool crtc_needs_colormap(const struct drm_crtc_state *crtc_state)
{
	struct drm_encoder *encoder = NULL;
	struct drm_device *dev = crtc_state->crtc->dev;
	bool processable_encoder = false;
	const struct exynos_drm_crtc *exynos_crtc = to_exynos_crtc(crtc_state->crtc);

	drm_for_each_encoder_mask(encoder, dev, crtc_state->encoder_mask)
		if (encoder->encoder_type == DRM_MODE_ENCODER_DSI ||
			encoder->encoder_type == DRM_MODE_ENCODER_TMDS)
			processable_encoder = true;

	return ((crtc_state->plane_mask & ~exynos_crtc->rcd_plane_mask) == 0) &&
	       crtc_state->color_mgmt_changed == false &&
	       processable_encoder == true;
}

static inline bool is_sr_active_changed(struct drm_crtc *crtc,
				  struct drm_atomic_state *state)
{
	struct drm_crtc_state *old_crtc_state, *new_crtc_state;

	WARN_ON(!crtc);

	old_crtc_state = drm_atomic_get_old_crtc_state(state, crtc);
	new_crtc_state = drm_atomic_get_new_crtc_state(state, crtc);

	return new_crtc_state->self_refresh_active !=
				old_crtc_state->self_refresh_active;
}

static inline bool is_crtc_psr_enabled(struct drm_crtc *crtc,
				       struct drm_atomic_state *state)
{
	struct drm_crtc_state *new_crtc_state;

	WARN_ON(!crtc);
	new_crtc_state = drm_atomic_get_new_crtc_state(state, crtc);

	return is_sr_active_changed(crtc, state) && new_crtc_state->self_refresh_active;
}

static inline bool is_crtc_psr_disabled(struct drm_crtc *crtc,
					struct drm_atomic_state *state)
{
	struct drm_crtc_state *new_crtc_state;

	WARN_ON(!crtc);
	new_crtc_state = drm_atomic_get_new_crtc_state(state, crtc);

	return is_sr_active_changed(crtc, state) && !new_crtc_state->self_refresh_active;
}

static inline u32 is_dual_blender_by_mode(struct drm_display_mode *mode)
{
	return (mode->hdisplay > 5120) ? 1 : 0;
}

void exynos_crtc_get_crc_data(struct exynos_drm_crtc *exynos_crtc);
void exynos_crtc_set_crc_state(struct exynos_drm_crtc *exynos_crtc, int crc_state);

struct drm_encoder*
crtc_get_encoder(const struct drm_crtc_state *crtc_state, u32 encoder_type);
struct drm_connector*
crtc_get_conn(const struct drm_crtc_state *crtc_state, u32 conn_type);
struct exynos_drm_connector *
crtc_get_exynos_conn(const struct drm_crtc_state *crtc_state, int conn_type);
void
exynos_crtc_wait_present_time(struct exynos_drm_crtc_state * exynos_crtc_state);

static inline struct exynos_drm_crtc_state *
exynos_drm_atomic_get_new_crtc_state(const struct drm_atomic_state *state,
		struct exynos_drm_crtc *exynos_crtc)
{
	struct drm_crtc_state *crtc_state = drm_atomic_get_new_crtc_state(state,
			&exynos_crtc->base);

	return to_exynos_crtc_state(crtc_state);
}

static inline struct exynos_drm_crtc_state *
exynos_drm_atomic_get_old_crtc_state(const struct drm_atomic_state *state,
		struct exynos_drm_crtc *exynos_crtc)
{
	struct drm_crtc_state *crtc_state = drm_atomic_get_old_crtc_state(state,
			&exynos_crtc->base);

	return to_exynos_crtc_state(crtc_state);
}
#endif
