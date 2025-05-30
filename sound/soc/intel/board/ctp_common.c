/*
 *  ctp_common.c - ASoc Machine driver for Intel Clovertrail MID platform
 *
 *  Copyright (C) 2011-13 Intel Corp
 *  Author: KP Jeeja<jeeja.kp@intel.com>
 *  Author: Vaibhav Agarwal <vaibhav.agarwal@intel.com>
 *  Author: Dharageswari.R<dharageswari.r@intel.com>
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#define DEBUG 1

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>
#include <linux/gpio.h>
#include <linux/rpmsg.h>
#include <linux/mod_devicetable.h>
#include <asm/intel_mid_gpadc.h>
#include <asm/intel_scu_pmic.h>
#include <asm/intel_scu_ipcutil.h>
#include <asm/intel_mid_rpmsg.h>
#include <asm/intel_mid_remoteproc.h>
#include <asm/platform_ctp_audio.h>
#include <sound/pcm.h>
#include <sound/jack.h>
#include <sound/soc.h>
#include "ctp_common.h"
#include <linux/input.h>

/* Headset jack detection gpios func(s) */
#define HPDETECT_POLL_INTERVAL  msecs_to_jiffies(750)  /* 750ms */
#define HS_DET_RETRY	1

//TC_Hsu : Add for report headset status to mid layer
//extern void mid_headset_report(int state);
void mid_headset_report(int state) {
	return;
}
extern long latest_resume_time(void);
struct timeval jack_bp_timeval;
static long jack_bp_time, jack_bp_latest_event_tv;

struct snd_soc_card snd_soc_card_ctp = {
	.name = "cloverview_audio",
	.set_bias_level = ctp_set_bias_level,
	.set_bias_level_post = ctp_set_bias_level_post,
};

unsigned int rates_8000_16000[] = {
	8000,
	16000,
};

struct snd_pcm_hw_constraint_list constraints_8000_16000 = {
	.count = ARRAY_SIZE(rates_8000_16000),
	.list = rates_8000_16000,
};

unsigned int rates_48000[] = {
	48000,
};

struct snd_pcm_hw_constraint_list constraints_48000 = {
	.count  = ARRAY_SIZE(rates_48000),
	.list   = rates_48000,
};

unsigned int rates_16000[] = {
	16000,
};

struct snd_pcm_hw_constraint_list constraints_16000 = {
	.count  = ARRAY_SIZE(rates_16000),
	.list   = rates_16000,
};

static struct snd_soc_jack_gpio hs_gpio[] = {
	[CTP_HSDET_GPIO] = {
		.name = "gpio_plugdet",
		.report = SND_JACK_HEADSET,
		.jack_status_check = ctp_soc_jack_gpio_detect,
		.irq_flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
	},
#ifdef CONFIG_TF103CG
	[CTP_BTN_GPIO] = {
		.name = "HOOK_DET_N",
		.report = SND_JACK_HEADSET | SND_JACK_BTN_0,
		.debounce_time = 70,
		.jack_status_check = ctp_soc_jack_gpio_detect_bp,
		.irq_flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
	},
#else
		[CTP_BTN_GPIO] = {
		.name = "HOOK_DET",
		.report = SND_JACK_HEADSET | SND_JACK_BTN_0,
		.debounce_time = 20,
		.jack_status_check = ctp_soc_jack_gpio_detect_bp,
		.irq_flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
	},
#endif	
};

int ctp_startup_probe(struct snd_pcm_substream *substream)
{
	pr_debug("%s - applying rate constraint\n", __func__);
	snd_pcm_hw_constraint_list(substream->runtime, 0,
					SNDRV_PCM_HW_PARAM_RATE,
					&constraints_48000);
	return 0;
}

int ctp_startup_asp(struct snd_pcm_substream *substream)
{
	pr_debug("%s - applying rate constraint\n", __func__);
	snd_pcm_hw_constraint_list(substream->runtime, 0,
				SNDRV_PCM_HW_PARAM_RATE,
				&constraints_48000);
	return 0;
}

int ctp_startup_bt_xsp(struct snd_pcm_substream *substream)
{
	pr_debug("%s - applying rate constraint\n", __func__);
	snd_pcm_hw_constraint_list(substream->runtime, 0,
				SNDRV_PCM_HW_PARAM_RATE,
				&constraints_8000_16000);
	return 0;
}
int get_ssp_bt_sco_master_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card =  snd_kcontrol_chip(kcontrol);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	struct comms_mc_private *ctl = &(ctx->comms_ctl);

	ucontrol->value.integer.value[0] = ctl->ssp_bt_sco_master_mode;
	return 0;
}
EXPORT_SYMBOL_GPL(get_ssp_bt_sco_master_mode);

int set_ssp_bt_sco_master_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card =  snd_kcontrol_chip(kcontrol);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	struct comms_mc_private *ctl = &(ctx->comms_ctl);

	if (ucontrol->value.integer.value[0] == ctl->ssp_bt_sco_master_mode)
		return 0;

	ctl->ssp_bt_sco_master_mode = ucontrol->value.integer.value[0];

	return 0;
}
EXPORT_SYMBOL_GPL(set_ssp_bt_sco_master_mode);

int get_ssp_voip_master_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card =  snd_kcontrol_chip(kcontrol);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	struct comms_mc_private *ctl = &(ctx->comms_ctl);

	ucontrol->value.integer.value[0] = ctl->ssp_voip_master_mode;
	return 0;
}
EXPORT_SYMBOL_GPL(get_ssp_voip_master_mode);

int set_ssp_voip_master_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card =  snd_kcontrol_chip(kcontrol);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	struct comms_mc_private *ctl = &(ctx->comms_ctl);

	if (ucontrol->value.integer.value[0] == ctl->ssp_voip_master_mode)
		return 0;

	ctl->ssp_voip_master_mode = ucontrol->value.integer.value[0];

	return 0;
}
EXPORT_SYMBOL_GPL(set_ssp_voip_master_mode);

int get_ssp_modem_master_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card =  snd_kcontrol_chip(kcontrol);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	struct comms_mc_private *ctl = &(ctx->comms_ctl);

	ucontrol->value.integer.value[0] = ctl->ssp_modem_master_mode;
	return 0;
}
EXPORT_SYMBOL_GPL(get_ssp_modem_master_mode);

int set_ssp_modem_master_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card =  snd_kcontrol_chip(kcontrol);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	struct comms_mc_private *ctl = &(ctx->comms_ctl);

	if (ucontrol->value.integer.value[0] == ctl->ssp_modem_master_mode)
		return 0;

	ctl->ssp_modem_master_mode = ucontrol->value.integer.value[0];

	return 0;
}
EXPORT_SYMBOL_GPL(set_ssp_modem_master_mode);

int ctp_set_clk_fmt(struct snd_soc_dai *codec_dai, struct ctp_clk_fmt *clk_fmt)
{
	int ret;

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, clk_fmt->fmt);

	if (ret < 0) {
		pr_err("can't set codec DAI configuration %d\n", ret);
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(codec_dai, clk_fmt->clk_id,
					clk_fmt->freq, clk_fmt->dir);
	if (ret < 0) {
		pr_err("can't set codec clock %d\n", ret);
		return ret;
	}

	return 0;
}

/* Board specific codec bias level control */
int ctp_set_bias_level(struct snd_soc_card *card,
		struct snd_soc_dapm_context *dapm,
		enum snd_soc_bias_level level)
{
	struct snd_soc_codec *codec;

	/* Clock management is done only if there is an associated codec
	 * to dapm context and if this not the dummy codec
	 */
	if (dapm->codec) {
		codec = dapm->codec;
		if (!strcmp(codec->name, "snd-soc-dummy"))
			return 0;
	} else {
		//pr_debug("In %s dapm context has no associated codec or it is dummy codec.", __func__);
		return 0;
	}

	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
	case SND_SOC_BIAS_STANDBY:
		if (card->dapm.bias_level == SND_SOC_BIAS_OFF)
			intel_scu_ipc_set_osc_clk0(true, CLK0_MSIC);
		card->dapm.bias_level = level;
		break;
	case SND_SOC_BIAS_OFF:
		/* OSC clk will be turned OFF after processing
		 * codec->dapm.bias_level = SND_SOC_BIAS_OFF.
		 */
		break;
	default:
		pr_err("%s: Invalid bias level=%d\n", __func__, level);
		return -EINVAL;
		break;
	}
	pr_debug("card(%s)->bias_level %u\n", card->name,
			card->dapm.bias_level);

	return 0;
}

int ctp_set_bias_level_post(struct snd_soc_card *card,
		struct snd_soc_dapm_context *dapm,
		enum snd_soc_bias_level level)
{
	struct snd_soc_codec *codec;

	/* Clock management is done only if there is an associated codec
	 * to dapm context and if this not the dummy codec
	 */
	if (dapm->codec) {
		codec = dapm->codec;
		if (!strcmp(codec->name, "snd-soc-dummy"))
			return 0;
	} else {
		//pr_debug("In %s dapm context has no associated codec or it is dummy codec.", __func__);
		return 0;
	}

	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
	case SND_SOC_BIAS_STANDBY:
		/* Processing already done during set_bias_level()
		 * callback. No action required here.
		 */
		break;
	case SND_SOC_BIAS_OFF:
		if (codec->dapm.bias_level != SND_SOC_BIAS_OFF)
			break;
		intel_scu_ipc_set_osc_clk0(false, CLK0_MSIC);
		card->dapm.bias_level = level;
		break;
	default:
		pr_err("%s: Invalid bias level=%d\n", __func__, level);
		return -EINVAL;
		break;
	}
	pr_debug("%s:card(%s)->bias_level %u\n", __func__, card->name,
			card->dapm.bias_level);
	return 0;
}
/*
static int set_mic_bias(struct snd_soc_jack *jack,
			const char *bias_widget, bool enable)
{
	struct snd_soc_codec *codec = jack->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	if (enable)
		snd_soc_dapm_force_enable_pin(dapm, bias_widget);
	else
		snd_soc_dapm_disable_pin(dapm, bias_widget);

	snd_soc_dapm_sync(&codec->dapm);

	return 0;
}
*/
int ctp_amp_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *k, int event)
{
	int ret;

	if (SND_SOC_DAPM_EVENT_ON(event)) {
		/*Enable  IHFAMP_SD_N  GPIO */
		ret = intel_scu_ipc_iowrite8(GPIOHVCTL, GPIO_AMP_ON);
		if (ret)
			pr_err("write of  failed, err %d\n", ret);
	} else {
		/*Disable  IHFAMP_SD_N  GPIO */
		ret = intel_scu_ipc_iowrite8(GPIOHVCTL, GPIO_AMP_OFF);
		if (ret)
			pr_err("write of  failed, err %d\n", ret);
	}
	return 0;
}

static inline void set_bp_interrupt(struct ctp_mc_private *ctx, bool enable)
{
	if (!enable) {
		if (!atomic_dec_return(&ctx->bpirq_flag)) {
			pr_debug("Disable %d interrupt line\n", ctx->bpirq);
			disable_irq_nosync(ctx->bpirq);
		} else
			atomic_inc(&ctx->bpirq_flag);
	} else {
		if (atomic_inc_return(&ctx->bpirq_flag) == 1) {
			/* If BP intr not enabled */
			pr_debug("Enable %d interrupt line\n", ctx->bpirq);
			enable_irq(ctx->bpirq);
		} else
			atomic_dec(&ctx->bpirq_flag);
	}
}

void cancel_all_work(struct ctp_mc_private *ctx)
{
	struct snd_soc_jack_gpio *gpio;
	cancel_delayed_work_sync(&ctx->jack_work_insert);
	cancel_delayed_work_sync(&ctx->jack_work_remove);
	gpio = &hs_gpio[CTP_BTN_GPIO];
	cancel_delayed_work_sync(&gpio->work);
}

int ctp_soc_jack_gpio_detect(void)
{
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_HSDET_GPIO];
	struct snd_soc_jack *jack = gpio->jack;
	struct ctp_mc_private *ctx =
		container_of(jack, struct ctp_mc_private, ctp_jack);

	int enable;

	/* During jack removal, spurious BP interrupt may occur.
	 * Better to disable interrupt until jack insert/removal stabilize.
	 * Also cancel the BP and jack_work if already sceduled */
	cancel_all_work(ctx);
	//set_bp_interrupt(ctx, false);
	enable = gpio_get_value(gpio->gpio);
#ifdef CONFIG_TF103CG
#else
	if (gpio->invert)
		
#endif		
	pr_debug("%s:gpio->%d=0x%d\n", __func__, gpio->gpio, enable);
#ifdef CONFIG_TF103CG
	enable = !enable;
#else
#endif
	if (enable) {
		atomic_set(&ctx->hs_det_retry, HS_DET_RETRY);
		schedule_delayed_work(&ctx->jack_work_insert,
					HPDETECT_POLL_INTERVAL);
	} else
		schedule_delayed_work(&ctx->jack_work_remove,
					HPDETECT_POLL_INTERVAL);
#ifdef CONFIG_HAS_WAKELOCK
	/*
	 * Take wakelock for one second to give time for the detection
	 * to finish. Jack detection is happening rarely so this doesn't
	 * have big impact to power consumption.
	 */
	wake_lock_timeout(ctx->jack_wake_lock,
			HPDETECT_POLL_INTERVAL + msecs_to_jiffies(50));
#endif

	/* Report old status */
	return jack->status;
}

/* Jack insert delayed work */
void headset_insert_poll(struct work_struct *work)
{
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_HSDET_GPIO];
	struct snd_soc_jack *jack = gpio->jack;
	struct snd_soc_codec *codec = jack->codec;
	struct ctp_mc_private *ctx =
		container_of(jack, struct ctp_mc_private, ctp_jack);

	int enable, status;
	unsigned int mask = SND_JACK_HEADSET;

	enable = gpio_get_value(gpio->gpio);
#ifdef CONFIG_TF103CG	
#else
	if (gpio->invert)
		enable = !enable;
	if (!enable) {
		pr_err("%s:gpio status = 0x%d\n", __func__, enable);
		return;
	}
#endif
	pr_debug("%s: Current jack status = 0x%x\n", __func__, jack->status);
#ifdef CONFIG_HAS_WAKELOCK
		/* Give sufficient time for the detection to propagate*/
		wake_lock_timeout(ctx->jack_wake_lock,
				1000 + msecs_to_jiffies(50));
#endif
	//set_mic_bias(jack, "micbias1", true);
	//msleep(ctx->ops->micsdet_debounce);
	status = ctx->ops->hp_detection(codec, jack, enable);
	if (status == SND_JACK_HEADSET) {
		//set_bp_interrupt(ctx, true);
		ctx->headset_plug_flag = true;
		mask = SND_JACK_HEADSET | SND_JACK_BTN_0;
	}
	if (jack->status != status) {
		pr_debug("%s: jack report status = 0x%x, mask = 0x%x", __func__, status, mask);
		snd_soc_jack_report(jack, status, mask);
	}

	/*
	 * At this point the HS may be half inserted and still be
	 * detected as HP, so recheck after HPDETECT_POLL_INTERVAL
	 */
	if (!atomic_dec_and_test(&ctx->hs_det_retry) &&
			status != SND_JACK_HEADSET) {
		pr_debug("HS Jack detect Retry %d\n",
				atomic_read(&ctx->hs_det_retry));
#ifdef CONFIG_HAS_WAKELOCK
		/* Give sufficient time for the detection to propagate*/
		wake_lock_timeout(ctx->jack_wake_lock,
				HPDETECT_POLL_INTERVAL + msecs_to_jiffies(50));
#endif
		schedule_delayed_work(&ctx->jack_work_insert,
					HPDETECT_POLL_INTERVAL);
	}

	if (status == SND_JACK_HEADPHONE)
		mid_headset_report(2); //Report to mid layer, headset without mic plug-in.
	else if (status == SND_JACK_HEADSET)
		mid_headset_report(1); //Report to mid layer, headset wich mic plug-in.
	else
		mid_headset_report(0); //Report to mid layer, headset plug-out.

	pr_debug("%s: status 0x%x\n", __func__, status);
}

/* Jack remove delayed work */
void headset_remove_poll(struct work_struct *work)
{
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_HSDET_GPIO];
	struct snd_soc_jack *jack = gpio->jack;
	struct snd_soc_codec *codec = jack->codec;
	struct ctp_mc_private *ctx =
		container_of(jack, struct ctp_mc_private, ctp_jack);

	int enable, status;
	unsigned int mask = SND_JACK_HEADSET | SND_JACK_BTN_0;

	enable = gpio_get_value(gpio->gpio);
#ifdef CONFIG_TF103CG
#else
	if (gpio->invert)
		enable = !enable;
	if (enable) {
		pr_err("%s:gpio status = 0x%d\n", __func__, enable);
		return;
	}
#endif
	pr_debug("%s: Current jack status = 0x%x\n", __func__, jack->status);
	status = ctx->ops->hp_detection(codec, jack, enable);
	//set_bp_interrupt(ctx, false);
	ctx->headset_plug_flag = false;
	//set_mic_bias(jack, "micbias1", false);

	if (jack->status != status)
		snd_soc_jack_report(jack, status, mask);

	mid_headset_report(0); //Report to mid layer, no headset plug-in.
	
	pr_debug("%s: status 0x%x\n", __func__, status);
}
#ifdef CONFIG_TF103CG
static irqreturn_t uart_disable_hs(int irq, void *dev_id)
{
	int status;
	status = gpio_get_value(get_gpio_by_name("PHONE_UART_DET#"));
	if (status == 8192)
		enable_irq(gpio_to_irq(get_gpio_by_name("gpio_plugdet")));
	else   
		disable_irq(gpio_to_irq(get_gpio_by_name("gpio_plugdet")));
	return IRQ_HANDLED;
};
#endif
int ctp_soc_jack_gpio_detect_bp(void)
{
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_BTN_GPIO];
	int enable, hs_status, status;
	struct snd_soc_jack *jack = gpio->jack;
	struct snd_soc_codec *codec = jack->codec;
	unsigned int mask = SND_JACK_BTN_0 | SND_JACK_HEADSET;
	struct ctp_mc_private *ctx =
		container_of(jack, struct ctp_mc_private, ctp_jack);

	pr_debug("enter %s\n", __func__);
	status = jack->status;
	enable = gpio_get_value(gpio->gpio);
	if (gpio->invert)
		enable = !enable;

	/* Check for headset status before processing interrupt */
	gpio = &hs_gpio[CTP_HSDET_GPIO];
	hs_status = gpio_get_value(gpio->gpio);
	if (gpio->invert)
		hs_status = !hs_status;
	pr_debug("in %s hook detect = 0x%x, headset detect = 0x%x\n", __func__, enable, hs_status);
	pr_debug("in %s jack status = 0x%x\n", __func__, jack->status);
	
#if defined(CONFIG_TF103CG) || defined(CONFIG_FE170CG)
	hs_status = !hs_status;
#endif
	if (((jack->status & SND_JACK_HEADSET) == SND_JACK_HEADSET)
						&& (hs_status)) {
    do_gettimeofday(&jack_bp_timeval);
    jack_bp_time = ((long)jack_bp_timeval.tv_sec) * 1000 + (long)jack_bp_timeval.tv_usec / 1000;
	  jack_bp_latest_event_tv = jack_bp_time - latest_resume_time();

		/* HS present, process the interrupt */
		//if (enable) {
			/* Jack removal might be in progress, check interrupt status
			 * before proceeding for button press detection */
			if (!atomic_dec_return(&ctx->bpirq_flag)) {
				status = ctx->ops->bp_detection(codec, jack, enable);
				if (status == mask) {
				    msleep(100);

				    gpio = &hs_gpio[CTP_BTN_GPIO];
				    enable = gpio_get_value(gpio->gpio);
	                       if (gpio->invert)
		                     enable = !enable;

				    status = ctx->ops->bp_detection(codec, jack, enable);		 

                              gpio = &hs_gpio[CTP_HSDET_GPIO];
				    hs_status = gpio_get_value(gpio->gpio);
	                            if (gpio->invert)
		                          hs_status = !hs_status;  
                                  #if defined(CONFIG_TF103CG) || defined(CONFIG_FE170CG)
                                      hs_status = !hs_status;
                                  #endif		  
								
				    if (((jack->status & SND_JACK_HEADSET) == SND_JACK_HEADSET) && (hs_status)) {
                                  if ((status == mask) && (ctx->btn_press_flag == false)) { 
					      ctx->btn_press_flag = true;
                                  }else if((status != mask) && (ctx->btn_press_flag == false)){
				             snd_soc_jack_report(jack, mask, mask);	
					  }                                
				    }
				} else {
					if (!(ctx->btn_press_flag)) {					
#ifdef CONFIG_FE170CG
            if (jack_bp_latest_event_tv > 0 && jack_bp_latest_event_tv < 200) //jessica_chou: add for ESD test
#endif
						snd_soc_jack_report(jack, mask, mask);

						pr_debug("%s:release event without press event, suspend?", __func__);
					}
					ctx->btn_press_flag = false;
				}
				atomic_inc(&ctx->bpirq_flag);
			} else
				atomic_inc(&ctx->bpirq_flag);
		//} else {
			//pr_debug("%s:Invalid BP interrupt\n", __func__);
		//}
	} else {
		pr_debug("%s:Spurious BP interrupt : jack_status 0x%x, HS_status 0x%x\n",
				__func__, jack->status, hs_status);
		//set_mic_bias(jack, "micbias1", false);
		/* Disable Button_press interrupt if no Headset */
		//set_bp_interrupt(ctx, false);
	}
	pr_debug("leave %s: status 0x%x\n", __func__, status);

	return status;
}


#ifdef CONFIG_PM

static int snd_ctp_prepare(struct device *dev)
{
	struct snd_soc_card *card = dev_get_drvdata(dev);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	pr_debug("In %s device name\n", __func__);

	/* switch the mclk to the lowpower mode */
	if (ctx->headset_plug_flag && !ctx->voice_call_flag) {
		if (ctx->ops->mclk_switch) {
			ctx->ops->mclk_switch(dev, false);
			/* Decrease the OSC clk to 4.8Mhz when suspend */
			intel_scu_ipc_osc_clk(OSC_CLK_AUDIO, 4800);
		}
	}
	return snd_soc_suspend(dev);
}
static void snd_ctp_complete(struct device *dev)
{
	struct snd_soc_card *card = dev_get_drvdata(dev);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	pr_debug("In %s\n", __func__);
	/* switch the mclk to the normal mode */
	if (ctx->headset_plug_flag && !ctx->voice_call_flag) {
		if (ctx->ops->mclk_switch) {
			/* recovery the OSC clk to 19.2Mhz when resume */
			intel_scu_ipc_osc_clk(OSC_CLK_AUDIO, 19200);
			ctx->ops->mclk_switch(dev, true);
		}
	}
	snd_soc_resume(dev);
	
}

static int snd_ctp_poweroff(struct device *dev)
{
	pr_debug("In %s\n", __func__);
	return snd_soc_poweroff(dev);
}

#else
#define snd_ctp_suspend NULL
#define snd_ctp_resume NULL
#define snd_ctp_poweroff NULL
#endif

static void free_jack_wake_lock(struct ctp_mc_private *ctx)
{
	if (!ctx->ops->jack_support)
		return;
#ifdef CONFIG_HAS_WAKELOCK
	if (wake_lock_active(ctx->jack_wake_lock))
		wake_unlock(ctx->jack_wake_lock);
	wake_lock_destroy(ctx->jack_wake_lock);
#endif
}

static void snd_ctp_unregister_jack(struct ctp_mc_private *ctx,
				struct platform_device *pdev)
{
	if (!ctx->ops->jack_support)
		return;
	cancel_delayed_work_sync(&ctx->jack_work_insert);
	cancel_delayed_work_sync(&ctx->jack_work_remove);
	free_jack_wake_lock(ctx);
	snd_soc_jack_free_gpios(&ctx->ctp_jack, 2, ctx->hs_gpio_ops);
}

static int snd_ctp_mc_remove(struct platform_device *pdev)
{
	struct snd_soc_card *soc_card = platform_get_drvdata(pdev);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(soc_card);

	pr_debug("In %s\n", __func__);
	snd_ctp_unregister_jack(ctx, pdev);
	snd_soc_card_set_drvdata(soc_card, NULL);
	snd_soc_unregister_card(soc_card);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static int snd_ctp_jack_init(struct snd_soc_pcm_runtime *runtime,
						bool jack_supported)
{
	int ret, irq;
#ifdef CONFIG_TF103CG
	int gpio1, status;
#endif
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(runtime->card);
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_BTN_GPIO];
	struct snd_soc_codec *codec = runtime->codec;

	if (!jack_supported)
		return 0;

	/* Setup the HPDET timer */
	INIT_DELAYED_WORK(&ctx->jack_work_insert, headset_insert_poll);
	INIT_DELAYED_WORK(&ctx->jack_work_remove, headset_remove_poll);

	/* Headset and button jack detection */
	ret = snd_soc_jack_new(codec, "Intel MID Audio Jack",
			SND_JACK_HEADSET | SND_JACK_BTN_0, &ctx->ctp_jack);
	if (ret) {
		pr_err("jack creation failed\n");
		return ret;
	}
	ret = snd_soc_jack_add_gpios(&ctx->ctp_jack, 2, ctx->hs_gpio_ops);
	if (ret) {
		pr_err("adding jack GPIO failed\n");
		return ret;
	}
	irq = gpio_to_irq(gpio->gpio);
	if (irq < 0) {
		pr_err("%d:Failed to map gpio_to_irq\n", irq);
		return irq;
	}

	/*set key */
	ret = snd_jack_set_key((&ctx->ctp_jack)->jack, SND_JACK_HEADSET | SND_JACK_BTN_0, BTN_MISC);

	/* Disable Button_press interrupt if no Headset */
	pr_err("Disable %d interrupt line\n", irq);
	disable_irq_nosync(irq);
	atomic_set(&ctx->bpirq_flag, 0);
	atomic_set(&ctx->hs_det_retry, HS_DET_RETRY);
#ifdef CONFIG_TF103CG
	gpio1=get_gpio_by_name("PHONE_UART_DET#");
	ret=request_irq(gpio_to_irq(gpio1),uart_disable_hs,IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,"UART",NULL);
	status=gpio_get_value(get_gpio_by_name("PHONE_UART_DET#"));
	if(!status)
		disable_irq(gpio_to_irq(get_gpio_by_name("gpio_plugdet")));
#endif         
	return 0;
}


int snd_ctp_init(struct snd_soc_pcm_runtime *runtime)
{
	int ret;
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(runtime->card);

	ret = ctx->ops->ctp_init(runtime);
	if (ret) {
		pr_err("CTP init returned failure\n");
		return ret;
	}
	return snd_ctp_jack_init(runtime, ctx->ops->jack_support);
}

int snd_ctp_register_jack_data(struct platform_device *pdev,
					struct ctp_mc_private *ctx)
{
	struct ctp_audio_platform_data *pdata = pdev->dev.platform_data;
	int ret_val = 0;
	if (!ctx->ops->jack_support)
		return 0;
#ifdef CONFIG_HAS_WAKELOCK
	ctx->jack_wake_lock =
		devm_kzalloc(&pdev->dev, sizeof(*(ctx->jack_wake_lock)), GFP_ATOMIC);
	if (!ctx->jack_wake_lock) {
		pr_err("allocation failed for wake_lock\n");
		return -ENOMEM;
	}
	wake_lock_init(ctx->jack_wake_lock, WAKE_LOCK_SUSPEND,
			"jack_detect");
#endif
	if (pdata->codec_gpio_hsdet >= 0 && pdata->codec_gpio_button >= 0) {
		hs_gpio[CTP_HSDET_GPIO].gpio = pdata->codec_gpio_hsdet;
		hs_gpio[CTP_BTN_GPIO].gpio = pdata->codec_gpio_button;
		ret_val = gpio_to_irq(hs_gpio[CTP_BTN_GPIO].gpio);
		if (ret_val < 0) {
			pr_err("%d:Failed to map button irq\n", ret_val);
			return ret_val;
		}
		ctx->bpirq = ret_val;
	}
	ctx->hs_gpio_ops = hs_gpio;
	return 0;
}
/* SoC card */
static int snd_ctp_mc_probe(struct platform_device *pdev)
{
	int ret_val = 0;
	struct ctp_mc_private *ctx;

	pr_debug("In %s\n", __func__);
	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_ATOMIC);
	if (!ctx) {
		pr_err("allocation failed\n");
		return -ENOMEM;
	}
	/* register the soc card */
	snd_soc_card_ctp.dev = &pdev->dev;

	ctx->ops = (struct snd_soc_machine_ops *)platform_get_device_id(pdev)->driver_data;
	//ctx->ops = ctp_get_rhb_ops();
	if (ctx->ops == NULL)
		return -EINVAL;

	ctx->ops->dai_link(&snd_soc_card_ctp);

	ret_val = snd_ctp_register_jack_data(pdev, ctx);
	if (ret_val)
		goto free_jack;

	snd_soc_card_set_drvdata(&snd_soc_card_ctp, ctx);
	ret_val = snd_soc_register_card(&snd_soc_card_ctp);
	if (ret_val) {
		pr_err("snd_soc_register_card failed %d\n", ret_val);
		goto free_jack;
	}

	platform_set_drvdata(pdev, &snd_soc_card_ctp);
	set_bp_interrupt(ctx, true);
	pr_debug("successfully exited probe\n");
	return ret_val;
free_jack:
	free_jack_wake_lock(ctx);
	return ret_val;
}

const struct dev_pm_ops snd_ctp_mc_pm_ops = {
	.prepare = snd_ctp_prepare,
	.complete = snd_ctp_complete,
	.poweroff = snd_ctp_poweroff,
};

static struct platform_device_id ctp_audio_ids[] = {
	{
		.name		= "ctp_audio",
		.driver_data	= (kernel_ulong_t)&ctp_rhb_ops,
	},
	/*
	{
		.name		= "ctp_vb_cs42l73",
		.driver_data	= (kernel_ulong_t)&ctp_vb_cs42l73_ops,
	},
	*/
	{ },
};
MODULE_DEVICE_TABLE(platform, ctp_audio_ids);

static struct platform_driver snd_ctp_mc_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "ctp_audio",
		.pm   = &snd_ctp_mc_pm_ops,
	},
	.probe = snd_ctp_mc_probe,
	.remove = snd_ctp_mc_remove,
	.id_table = ctp_audio_ids,
};

static int __init snd_ctp_driver_init(void)
{
	pr_info("In %s\n", __func__);
	return platform_driver_register(&snd_ctp_mc_driver);
}

static void snd_ctp_driver_exit(void)
{
	pr_debug("In %s\n", __func__);
	platform_driver_unregister(&snd_ctp_mc_driver);
}

static int snd_clv_rpmsg_probe(struct rpmsg_channel *rpdev)
{
	int ret = 0;

	if (rpdev == NULL) {
		pr_err("rpmsg channel not created\n");
		ret = -ENODEV;
		goto out;
	}

	dev_info(&rpdev->dev, "Probed snd_clv rpmsg device\n");

	ret = snd_ctp_driver_init();

out:
	return ret;
}

static void snd_clv_rpmsg_remove(struct rpmsg_channel *rpdev)
{
	snd_ctp_driver_exit();
	dev_info(&rpdev->dev, "Removed snd_clv rpmsg device\n");
}

static void snd_clv_rpmsg_cb(struct rpmsg_channel *rpdev, void *data,
				int len, void *priv, u32 src)
{
	dev_warn(&rpdev->dev, "unexpected, message\n");

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
			data, len,  true);
}

static struct rpmsg_device_id snd_clv_rpmsg_id_table[] = {
	{ .name = "rpmsg_msic_clv_audio" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, snd_clv_rpmsg_id_table);

static struct rpmsg_driver snd_clv_rpmsg = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= snd_clv_rpmsg_id_table,
	.probe		= snd_clv_rpmsg_probe,
	.callback	= snd_clv_rpmsg_cb,
	.remove		= snd_clv_rpmsg_remove,
};

static int __init snd_clv_rpmsg_init(void)
{
	return register_rpmsg_driver(&snd_clv_rpmsg);
}

late_initcall(snd_clv_rpmsg_init);

static void __exit snd_clv_rpmsg_exit(void)
{
	return unregister_rpmsg_driver(&snd_clv_rpmsg);
}
module_exit(snd_clv_rpmsg_exit);

