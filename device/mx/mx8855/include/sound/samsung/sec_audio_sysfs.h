#ifndef _SEC_AUDIO_SYSFS_H
#define _SEC_AUDIO_SYSFS_H

/*
 * 1 AMP project - mono(AMP_0)
 * 2 AMP project - L(AMP_0), R(AMP_1)
 * 3 AMP project - L(AMP_0), R(AMP_1), receiver(AMP_2)
 * 4 AMP project - FL(AMP_1), FR(AMP_2), RL(AMP_3), RR(AMP_4)
 */
enum amp_id {
	AMP_0,
	AMP_1,
	AMP_2,
	AMP_3,
	AMP_ID_MAX,
};

enum ready_state {
	NOT_SUPPORT = -1,
	INIT_FAIL,
	INIT_SUCCESS
};

struct sec_audio_sysfs_data {
	struct class *audio_class;
	struct device *jack_dev;
	struct device *codec_dev;
	struct device *amp_dev;
	struct device *adsp_dev;
	bool no_earjack;
	int (*get_jack_state)(void);
	int (*get_key_state)(void);
	int (*set_jack_state)(int);
	int (*get_mic_adc)(void);
	int (*get_codec_id_state)(void);
	int (*set_force_enable_antenna)(int);
	int (*get_antenna_state)(void);

	/* bigdata */
	unsigned int num_amp;
	int (*get_amp_temperature_max)(enum amp_id);
	int (*get_amp_temperature_keep_max)(enum amp_id);
	int (*get_amp_temperature_overcount)(enum amp_id);
	int (*get_amp_excursion_max)(enum amp_id);
	int (*get_amp_excursion_overcount)(enum amp_id);
	int (*get_amp_curr_temperature)(enum amp_id);
	int (*set_amp_surface_temperature)(enum amp_id, int);
	int (*get_amp_ready)(enum amp_id);
};

struct sec_audio_counts {
	int count;
	int count_sum;
};

struct sec_audio_count_data {
	struct sec_audio_counts aifecnt[AMP_ID_MAX];
	struct sec_audio_counts cifecnt;
	struct sec_audio_counts adsp_sr;
};

struct sec_audio_ready_data {
	int amp[AMP_ID_MAX];
};

#if IS_ENABLED(CONFIG_SND_SOC_SAMSUNG_AUDIO)
int audio_register_jack_select_cb(int (*set_jack) (int));
int audio_register_jack_state_cb(int (*jack_status) (void));
int audio_register_key_state_cb(int (*key_state) (void));
int audio_register_mic_adc_cb(int (*mic_adc) (void));
int audio_register_force_enable_antenna_cb(int (*force_enable_antenna) (int));
int audio_register_antenna_state_cb(int (*antenna_state) (void));
int audio_register_codec_id_state_cb(int (*codec_id_state) (void));
void send_codec_i2c_fail_ev(void);
void send_amp_ready_ev(enum amp_id, enum ready_state);
enum ready_state get_amp_ready_state(enum amp_id);

/* bigdata */
int audio_register_temperature_max_cb(int (*temperature_max) (enum amp_id));
int audio_register_temperature_keep_max_cb(int (*temperature_keep_max) (enum amp_id));
int audio_register_temperature_overcount_cb(int (*temperature_overcount) (enum amp_id));
int audio_register_excursion_max_cb(int (*excursion_max) (enum amp_id));
int audio_register_excursion_overcount_cb(int (*excursion_overcount) (enum amp_id));
int audio_register_curr_temperature_cb(int (*curr_temperature) (enum amp_id));
int audio_register_surface_temperature_cb(int (*surface_temperature) (enum amp_id, int temperature));
int audio_register_ready_cb(int (*ready) (enum amp_id));
void send_amp_i2c_fail_ev(int amp_id);
#else
inline int audio_register_jack_select_cb(int (*set_jack) (int))
{
	return -EACCES;
}

inline int audio_register_jack_state_cb(int (*jack_status) (void))
{
	return -EACCES;
}

inline int audio_register_key_state_cb(int (*key_state) (void))
{
	return -EACCES;
}

inline int audio_register_mic_adc_cb(int (*mic_adc) (void))
{
	return -EACCES;
}

inline int audio_register_force_enable_antenna_cb(int (*force_enable_antenna) (int))
{
	return -EACCES;
}

inline int audio_register_antenna_state_cb(int (*antenna_state) (void))
{
	return -EACCES;
}

inline int audio_register_codec_id_state_cb(int (*codec_id_state) (void))
{
	return -EACCES;
}

inline void send_codec_i2c_fail_ev(void)
{

}

inline void send_amp_ready_ev(enum amp_id id, enum ready_state state)
{

}

inline enum ready_state get_amp_ready_state(enum amp_id id)
{
	return NOT_SUPPORT;
}

inline int audio_register_temperature_max_cb(int (*temperature_max) (enum amp_id))
{
	return -EACCES;
}

inline int audio_register_temperature_keep_max_cb(int (*temperature_keep_max) (enum amp_id))
{
	return -EACCES;
}

inline int audio_register_temperature_overcount_cb(int (*temperature_overcount) (enum amp_id))
{
	return -EACCES;
}

inline int audio_register_excursion_max_cb(int (*excursion_max) (enum amp_id))
{
	return -EACCES;
}

inline int audio_register_excursion_overcount_cb(int (*excursion_overcount) (enum amp_id))
{
	return -EACCES;
}

inline int audio_register_curr_temperature_cb(int (*curr_temperature) (enum amp_id))
{
	return -EACCES;
}

inline int audio_register_surface_temperature_cb(int (*surface_temperature) (enum amp_id, int temperature))
{
	return -EACCES;
}

inline int audio_register_ready_cb(int (*ready) (enum amp_id))
{
	return -EACCES;
}

inline void send_amp_i2c_fail_ev(int amp_id)
{

}
#endif

#endif /* _SEC_AUDIO_SYSFS_H */
