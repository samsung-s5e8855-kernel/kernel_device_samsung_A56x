// SPDX-License-Identifier: GPL-2.0
/*
 * Samsung Specific feature
 *
 * Copyright (C) 2024 Samsung Electronics Co., Ltd.
 *
 * Authors:
 * Storage Driver <storage.sec@samsung.com>
 */

#ifndef __UFS_SEC_FEATURE_H__
#define __UFS_SEC_FEATURE_H__

#include "../core/ufshcd-priv.h"
#include <linux/sched/clock.h>
#include <ufs/ufshci.h>
#include <linux/notifier.h>

/*unique number*/
#define UFS_UN_20_DIGITS 20
#define UFS_UN_MAX_DIGITS (UFS_UN_20_DIGITS + 1) //current max digit + 1

#define SCSI_UFS_TIMEOUT		(10 * HZ)
#define SERIAL_NUM_SIZE 7

#define UFS_S_INFO_SIZE 512
#define UFS_SHI_SIZE 256

#define HEALTH_DESC_PARAM_SEC_FLT 0x22
#define HEALTH_DESC_PARAM_KIC_FLT 0x11
#define HEALTH_DESC_PARAM_MIC_FLT 0x5
#define HEALTH_DESC_PARAM_SKH_FLT 0x5

struct ufs_vendor_dev_info {
	struct ufs_hba *hba;
	char unique_number[UFS_UN_MAX_DIGITS];
	u8 lt;
	u16 flt;
	u8 eli;
	unsigned int ic;
	char s_info[UFS_S_INFO_SIZE];
	char shi[UFS_SHI_SIZE];
	bool device_stuck;

	const struct ufs_sec_variant_ops *vops;
};

struct ufs_sec_cmd_info {
	u8 opcode;
	u32 lba;
	int transfer_len;
	u8 lun;
};

enum ufs_sec_wb_state {
	WB_OFF = 0,
	WB_ON
};

struct ufs_sec_wb_info {
	bool support;
	u64 state_ts;
	u64 enable_ms;
	u64 disable_ms;
	u64 amount_kb;
	u64 enable_cnt;
	u64 disable_cnt;
	u64 err_cnt;
};

struct ufs_sec_variant_ops {
	int	(*create_sysfs_dev)(struct ufs_hba *);
	int	(*mode_enter_upload)(void);
	void	(*exin_ufs)(char *str);
	void	(*abc_event)(char *str);
};

enum ufs_sec_log_str_t {
	UFS_SEC_CMD_SEND,
	UFS_SEC_CMD_COMP,
	UFS_SEC_QUERY_SEND,
	UFS_SEC_QUERY_COMP,
	UFS_SEC_NOP_SEND,
	UFS_SEC_NOP_COMP,
	UFS_SEC_TM_SEND,
	UFS_SEC_TM_COMP,
	UFS_SEC_TM_ERR,
	UFS_SEC_UIC_SEND,
	UFS_SEC_UIC_COMP,
};

static const char * const ufs_sec_log_str[] = {
	[UFS_SEC_CMD_SEND] = "scsi_send",
	[UFS_SEC_CMD_COMP] = "scsi_cmpl",
	[UFS_SEC_QUERY_SEND] = "query_send",
	[UFS_SEC_QUERY_COMP] = "query_cmpl",
	[UFS_SEC_NOP_SEND] = "nop_send",
	[UFS_SEC_NOP_COMP] = "nop_cmpl",
	[UFS_SEC_TM_SEND] = "tm_send",
	[UFS_SEC_TM_COMP] = "tm_cmpl",
	[UFS_SEC_TM_ERR] = "tm_err",
	[UFS_SEC_UIC_SEND] = "uic_send",
	[UFS_SEC_UIC_COMP] = "uic_cmpl",
};

struct ufs_sec_cmd_log_entry {
	const char *str;	/* ufs_sec_log_str */
	u8 lun;
	u8 cmd_id;
	u32 lba;
	int transfer_len;
	u8 idn;		/* used only for query idn */
	unsigned long outstanding_reqs;
	unsigned int tag;
	u64 tstamp;
};

#define UFS_SEC_CMD_LOGGING_MAX 200
#define UFS_SEC_CMD_LOGNODE_MAX 64
struct ufs_sec_cmd_log_info {
	struct ufs_sec_cmd_log_entry *entries;
	int pos;
};

struct ufs_sec_feature_info {
	struct ufs_vendor_dev_info *vdi;
	struct ufs_sec_wb_info *ufs_wb;
	struct ufs_sec_wb_info *ufs_wb_backup;
	struct ufs_sec_err_info *ufs_err;
	struct ufs_sec_err_info *ufs_err_backup;
	struct ufs_sec_err_info *ufs_err_hist;
	struct ufs_sec_cmd_log_info *ufs_cmd_log;

	struct ufs_sec_hcgc_info *ufs_hcgc;

	struct notifier_block reboot_notify;
	struct delayed_work noti_work;

	u32 ext_ufs_feature_sup;
	u32 vendor_spec_feature_sup;

	u32 last_ucmd;
	bool ucmd_complete;

	enum query_opcode last_qcmd;
	enum dev_cmd_type qcmd_type;
	bool qcmd_complete;
};

extern struct device *sec_ufs_node_dev;

/* call by vendor module */
void ufs_sec_config_features(struct ufs_hba *hba);
void ufs_sec_adjust_caps_quirks(struct ufs_hba *hba);
void ufs_sec_init_logging(struct device *dev);
void ufs_sec_set_features(struct ufs_hba *hba);
void ufs_sec_remove_features(struct ufs_hba *hba);
void ufs_sec_register_vendor_hooks(void);
void ufs_sec_print_err_info(struct ufs_hba *hba);
void ufs_sec_print_err(void);
void ufs_sec_inc_hwrst_cnt(void);
void ufs_sec_inc_op_err(struct ufs_hba *hba, enum ufs_event_type evt, void *data);
inline bool ufs_sec_is_err_cnt_allowed(void);

void ufs_sec_check_device_stuck(void);
void ufs_sec_get_health_desc(struct ufs_hba *hba);
bool ufs_sec_is_wb_supported(void);
int ufs_sec_wb_ctrl(bool enable);
void ufs_sec_wb_register_reset_notify(void *func);
void ufs_sec_register_callback(const struct ufs_sec_variant_ops *vops);

static inline int ufs_sec_vops_create_sysfs_dev(struct ufs_vendor_dev_info *vdi)
{
	if (vdi && vdi->hba && vdi->vops && vdi->vops->create_sysfs_dev)
		return vdi->vops->create_sysfs_dev(vdi->hba);

	return -ENODEV;
}

static inline int ufs_sec_vops_mode_enter_upload(struct ufs_vendor_dev_info *vdi)
{
	if (vdi && vdi->vops && vdi->vops->mode_enter_upload)
		return vdi->vops->mode_enter_upload();

	return 0;
}

static inline void ufs_sec_vops_exin_ufs(struct ufs_vendor_dev_info *vdi, char *str)
{
	if (vdi && vdi->vops && vdi->vops->exin_ufs)
		vdi->vops->exin_ufs(str);
}

static inline void ufs_sec_vops_abc_event(struct ufs_vendor_dev_info *vdi, char *str)
{
	if (vdi && vdi->vops && vdi->vops->abc_event)
		vdi->vops->abc_event(str);
}
#endif
