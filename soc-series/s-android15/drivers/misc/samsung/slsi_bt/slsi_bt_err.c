#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <pcie_scsc/scsc_mx.h>
#include "slsi_bt_err.h"
#include "slsi_bt_log.h"

static struct workqueue_struct	*wq;
static struct work_struct work;
static void (*handler)(int reason, bool lazy);
static atomic_t err_cnt;
static int recovery_st = SLSI_BT_RECOVERY_ST_NONE;

/* Store error history from first.
 * err_history[0] shows that is the first driver error after system booting.
 */
static int err_history[SLSI_BT_ERR_HISTORY_SIZE], sp, last_err;

static void set_recovery_state(int state)
{
	bool expected = true;

	BT_INFO("recovery state: %d -> %d\n", recovery_st, state);
	switch (recovery_st) {
	case SLSI_BT_RECOVERY_ST_NONE:
		expected = (state != SLSI_BT_RECOVERY_ST_HW_INPROGRESS);
		break;
	case SLSI_BT_RECOVERY_ST_HW_INPROGRESS:
		expected = (state != SLSI_BT_RECOVERY_ST_HW_RESET);
		break;
	case SLSI_BT_RECOVERY_ST_HW_RESET:
		expected = (state != SLSI_BT_RECOVERY_ST_USER_NOTIFIED);
		break;
	case SLSI_BT_RECOVERY_ST_USER_NOTIFIED:
		expected = (state != SLSI_BT_RECOVERY_ST_NONE);
	}
	if (!expected)
		BT_WARNING("unexpected change\n");
	recovery_st = state;
}

static void debug_error_history(void)
{
	int i = sp;
	int more = atomic_read(&err_cnt) - sp;

	for (i = 0; i < sp; i++)
		BT_DBG("error history[%d]:  %d\n", i, err_history[i]);
	if (more > 0)
		BT_DBG("history stack full. %d more error...\n", more);
}

static void slsi_bt_err_inform(int reason)
{
	switch (reason) {
	case SLSI_BT_ERR_MX_FAIL:
		set_recovery_state(SLSI_BT_RECOVERY_ST_HW_INPROGRESS);
		break;
	case SLSI_BT_ERR_MX_RESET:    /* SLSI_BT_ERR_MX_FAIL is recovered */
		set_recovery_state(SLSI_BT_RECOVERY_ST_HW_RESET);
		break;
	}

	atomic_inc(&err_cnt);
	if (sp < SLSI_BT_ERR_HISTORY_SIZE)
		err_history[sp++] = reason;

	BT_ERR("count: %d, reason 0x%2x\n", atomic_read(&err_cnt), reason);
	last_err = reason;
	debug_error_history();
}

static bool get_req_restart(int reason)
{
	switch (reason) {
	case SLSI_BT_ERR_MX_RESET: __attribute__((__fallthrough__));
	case SLSI_BT_UART_WRITE_FAIL:
		return true;
	default:
		return false;
	}
	return false;
}

static void handler_worker(struct work_struct *work)
{
	BT_INFO("error handler: %p\n", handler);
	if (handler) {
		handler(last_err, get_req_restart(last_err));
		if (recovery_st == SLSI_BT_RECOVERY_ST_HW_RESET)
			set_recovery_state(SLSI_BT_RECOVERY_ST_USER_NOTIFIED);
	}
	BT_INFO("done\n");
}

void slsi_bt_err(int reason)
{
	slsi_bt_err_inform(reason);
	if (wq && handler) {
		queue_work(wq, &work);

		/* collect hcf only in bcsp driver error */
		if ((reason&0xF0) != (SLSI_BT_ERR_MX_FAIL&0xF0))
			bt_hcf_collection_request();
	}
}

int slsi_bt_err_status(void)
{
	return handler ? atomic_read(&err_cnt) : 0;
}

bool slsi_bt_in_recovery_progress(void)
{
	if (!mxman_recovery_disabled())
		return recovery_st == SLSI_BT_RECOVERY_ST_HW_INPROGRESS;
	return false;
}

int slsi_bt_err_proc_show(struct seq_file *m, void *v)
{
	int i = 0;
	seq_puts(m, "\nError history: \n");
	seq_printf(m, "  Error status           = %d\n", slsi_bt_err_status());
	seq_printf(m, "  Recover status         = %d\n", recovery_st);
	seq_printf(m, "  Last error             = %d\n", last_err);
	seq_puts(m, "  Error History after module initialized\n");;
	for (i = 0; i < sp; i++)
		seq_printf(m, "    History stack[%d]       = %d\n", i,
			(unsigned int)err_history[i]);
	return 0;
}

static int force_crash_set(const char *val, const struct kernel_param *kp)
{
	int ret;
	u32 value;

	ret = kstrtou32(val, 0, &value);
	BT_DBG("ret=%d val=%s value=0x%X\n", ret, val, value);
	if (!ret && value == 0xDEADDEAD)
		slsi_bt_err(SLSI_BT_ERR_FORCE_CRASH);

	return ret;
}

static struct kernel_param_ops slsi_bt_force_crash_ops = {
	.set = force_crash_set,
	.get = NULL,
};

static struct kernel_param_ops slsi_bt_force_crash_ops;
module_param_cb(force_crash, &slsi_bt_force_crash_ops, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(force_crash, "Forces a crash of the Bluetooth driver");

void slsi_bt_err_reset(void)
{
	atomic_set(&err_cnt, 0);
	if (recovery_st != SLSI_BT_RECOVERY_ST_NONE)
		set_recovery_state(SLSI_BT_RECOVERY_ST_NONE);
}

int slsi_bt_err_init(void (*callback)(int reason, bool lazy))
{
	atomic_set(&err_cnt, 0);
	if (callback == NULL)
		return 0;

	wq = create_singlethread_workqueue("bt_error_handler");
	if (wq == NULL) {
		BT_ERR("Fail to create workqueue\n");
		return -ENOMEM;
	}

	if (recovery_st != SLSI_BT_RECOVERY_ST_NONE)
		set_recovery_state(SLSI_BT_RECOVERY_ST_NONE);

	INIT_WORK(&work, handler_worker);
	handler = callback;
	return 0;
}

void slsi_bt_err_deinit(void)
{
	flush_workqueue(wq);
	destroy_workqueue(wq);
	cancel_work_sync(&work);
	wq = NULL;
	handler = NULL;
}
