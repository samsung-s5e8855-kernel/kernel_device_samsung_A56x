/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __KUNIT_MOCK_LOAD_MANAGER_H__
#define __KUNIT_MOCK_LOAD_MANAGER_H__

#include "../load_manager.h"

#define slsi_lbm_register_rps_control(args...)			kunit_mock_slsi_lbm_register_rps_control(args)
#define slsi_lbm_register_tasklet(args...)			kunit_mock_slsi_lbm_register_tasklet(args)
#define slsi_lbm_unregister_io_saturation_control(args...)	kunit_mock_slsi_lbm_unregister_io_saturation_control(args)
#define slsi_lbm_register_workqueue(args...)			kunit_mock_slsi_lbm_register_workqueue(args)
#define slsi_lbm_cpuhp_offline_cb(args...)			kunit_mock_slsi_lbm_cpuhp_offline_cb(args)
#define slsi_lbm_register_io_saturation_control(args...)	kunit_mock_slsi_lbm_register_io_saturation_control(args)
#define slsi_lbm_netdev_deactivate(args...)			kunit_mock_slsi_lbm_netdev_deactivate(args)
#define slsi_lbm_unregister_cpu_affinity_control(args...)	kunit_mock_slsi_lbm_unregister_cpu_affinity_control(args)
#define slsi_lbm_unregister_rps_control(args...)		kunit_mock_slsi_lbm_unregister_rps_control(args)
#define slsi_lbm_register_napi(args...)				kunit_mock_slsi_lbm_register_napi(args)
#define slsi_lbm_cpuhp_online_cb(args...)			kunit_mock_slsi_lbm_cpuhp_online_cb(args)
#define slsi_lbm_unregister_bh(args...)				kunit_mock_slsi_lbm_unregister_bh(args)
#define slsi_lbm_register_cpu_affinity_control(args...)		kunit_mock_slsi_lbm_register_cpu_affinity_control(args)
#define slsi_lbm_deinit(args...)				kunit_mock_slsi_lbm_deinit(args)
#define slsi_lbm_init(args...)					kunit_mock_slsi_lbm_init(args)
#define slsi_lbm_freeze(args...)				kunit_mock_slsi_lbm_freeze(args)
#define slsi_lbm_run_bh(args...)				kunit_mock_slsi_lbm_run_bh(args)
#define slsi_lbm_set_property(args...)				kunit_mock_slsi_lbm_set_property(args)
#define slsi_lbm_setup(args...)					kunit_mock_slsi_lbm_setup(args)
#define slsi_lbm_netdev_activate(args...)			kunit_mock_slsi_lbm_netdev_activate(args)
#define slsi_lbm_get_hip_priv_from_work(args...)		kunit_mock_slsi_lbm_get_hip_priv_from_work(args)
#define slsi_lbm_state_change_for_affinity
#define slsi_lbm_get_napi_cpu(args...)				kunit_mock_slsi_lbm_get_napi_cpu(args)
#define slsi_lbm_register_link_change_cb(args...)		kunit_mock_slsi_lbm_register_link_change_cb(args)
#ifdef CONFIG_SCSC_WLAN_TX_API
#define slsi_lbm_disable_tx_napi()				kunit_mock_slsi_lbm_disable_tx_napi();
#define slsi_lbm_enable_tx_napi()				kunit_mock_slsi_lbm_enable_tx_napi();
#endif


static struct rps_ctrl_info *kunit_mock_slsi_lbm_register_rps_control(struct net_device *dev,
								      const int mid, const int high)
{
	return NULL;
}

static struct bh_struct *kunit_mock_slsi_lbm_register_tasklet(struct slsi_dev *sdev,
							      void (*tasklet_func)(unsigned long data),
							      int irq)
{
	struct bh_struct *bh_tmp = kzalloc(sizeof(*bh_tmp), GFP_ATOMIC);
	struct slsi_hip *hip;

	tasklet_init(&bh_tmp->bh_priv.tasklet.bh, NULL, (unsigned long)hip);
	bh_tmp->bh_priv.tasklet.bh_s = bh_tmp;
	return bh_tmp;
}

static void kunit_mock_slsi_lbm_unregister_io_saturation_control(struct bh_struct *bh)
{
	return;
}

static struct bh_struct *kunit_mock_slsi_lbm_register_workqueue(struct slsi_dev *sdev,
								void (*workqueue_func)(struct work_struct *data),
								int irq)
{
	struct bh_struct *bh_tmp = kzalloc(sizeof(*bh_tmp), GFP_ATOMIC);

	INIT_WORK(&bh_tmp->bh_priv.work.bh, NULL);
	bh_tmp->bh_priv.work.bh_s = bh_tmp;
	return bh_tmp;
}

static int kunit_mock_slsi_lbm_cpuhp_offline_cb(int cpu, void *data)
{
	return 0;
}

static struct io_saturatioin_ctrl_info *kunit_mock_slsi_lbm_register_io_saturation_control(struct bh_struct *bh)
{
	return NULL;
}

static int kunit_mock_slsi_lbm_netdev_deactivate(struct slsi_dev *sdev, struct net_device *dev,
						 struct netdev_vif *ndev_vif)
{
	return 0;
}

static void kunit_mock_slsi_lbm_unregister_cpu_affinity_control(struct slsi_dev *sdev, struct bh_struct *bh)
{
	return;
}

static void kunit_mock_slsi_lbm_unregister_rps_control(struct net_device *dev)
{
	return;
}

static struct bh_struct *kunit_mock_slsi_lbm_register_napi(struct slsi_dev *sdev,
							   int (*napi_poll)(struct napi_struct *, int),
							   int irq, int idx)
{
	if (napi_poll)
		return kzalloc(sizeof(struct bh_struct), GFP_KERNEL);

	return NULL;
}

static int kunit_mock_slsi_lbm_cpuhp_online_cb(int cpu, void *data)
{
	return 0;
}

static int kunit_mock_slsi_lbm_unregister_bh(struct bh_struct *bh)
{
	return 0;
}

static struct cpu_affinity_ctrl_info *kunit_mock_slsi_lbm_register_cpu_affinity_control(struct bh_struct *bh, int idx)
{
	return NULL;
}

static void kunit_mock_slsi_lbm_deinit(struct slsi_dev *sdev)
{
	return;
}

static void kunit_mock_slsi_lbm_init(struct slsi_dev *sdev)
{
	return;
}

static void kunit_mock_slsi_lbm_freeze(void)
{
	return;
}

static int kunit_mock_slsi_lbm_run_bh(struct bh_struct *bh)
{
	return 0;
}

static int kunit_mock_slsi_lbm_set_property(struct slsi_dev *sdev)
{
	return 0;
}

static void kunit_mock_slsi_lbm_setup(struct bh_struct *bh)
{
	return;
}

static int kunit_mock_slsi_lbm_netdev_activate(struct slsi_dev *sdev, struct net_device *dev)
{
	return 0;
}

static struct hip_priv *kunit_mock_slsi_lbm_get_hip_priv_from_work(struct work_struct *data)
{
	struct work_priv *work_priv;

	if (!data)
		return NULL;

	work_priv = container_of(data, struct work_priv, bh);

	return work_priv->bh_s->hip_priv;
}

static unsigned int kunit_mock_slsi_lbm_get_napi_cpu(int idx, u32 state)
{
	return 0;
}

static void kunit_mock_slsi_lbm_register_link_change_cb(struct slsi_dev *sdev)
{
}

#ifdef CONFIG_SCSC_WLAN_TX_API
static void kunit_mock_slsi_lbm_disable_tx_napi(void)
{
}

static void kunit_mock_slsi_lbm_enable_tx_napi(void)
{
}
#endif
#endif
