/* wisky/sound/wisky_codec_ES8388.c
 *
 * Copyright (C) 2011 wisky
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * LOG
 * ------------------------------------------------------------
 * V001:20110403 cd lan
 *	1.Modify for wisky MID project base on ES8388.c in rk29xx paltform
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>

#include <mach/iomux.h>
#include <mach/gpio.h>

#include "wisky_codec_es8388.h"


#define AUDIO_NAME "ES8388"
#define ES8388_VERSION "v1.0"

#define SPK_CON 		SPEAKER_CTRL_PIN

#if 0
//cd lan@20110925^fix for M818
int audio_trigger_flag=1;//0--audio off, 1---audio on
#if defined(CONFIG_ANX7150) || defined(WISKY_HDMI_ANX7150)
extern int hdmi_status;// 0---lcd. 1--hdmi enable
#endif
extern int audio_volume;
#endif

unsigned int system_mute;
unsigned int system_mute_state;


static struct snd_soc_codec *es8388_codec;
struct snd_soc_codec_device soc_codec_dev_es8388;

/* codec private data */
struct es8388_priv {
	struct snd_soc_codec codec;
	u16 reg_cache[ES8388_CACHEREGNUM];
	unsigned int sysclk;
};



#define err(format, arg...) \
	printk(KERN_ERR AUDIO_NAME ": " format "\n" , ## arg)
#define info(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#define warn(format, arg...) \
	printk(KERN_WARNING AUDIO_NAME ": " format "\n" , ## arg)

static int es8388_ini(struct snd_soc_codec *codec,
			   enum snd_soc_control_type control);
/*
 * es8388 register cache
 * We can't read the es8388 register space when we
 * are using 2 wire for device control, so we cache them instead.
 */
static const u16 es8388_reg[] = {
	0x06, 0x1C, 0xC3, 0xFC,  /*  0 *////0x0100 0x0180
	0xC0, 0x00, 0x00, 0x7C,  /*  4 */
	0x80, 0x00, 0x00, 0x06,  /*  8 */
	0x00, 0x06, 0x30, 0x30,  /* 12 */
	0xC0, 0xC0, 0x38, 0xB0,  /* 16 */
	0x32, 0x06, 0x00, 0x00,  /* 20 */
	0x06, 0x32, 0xC0, 0xC0,  /* 24 */
	0x08, 0x06, 0x1F, 0xF7,  /* 28 */
	0xFD, 0xFF, 0x1F, 0xF7,  /* 32 */
	0xFD, 0xFF, 0x00, 0x38,  /* 36 */
	0x38, 0x38, 0x38, 0x38,  /* 40 */
	0x38, 0x00, 0x00, 0x00,  /* 44 */
	0x00, 0x00, 0x00, 0x00,  /* 48 */
	0x00, 0x00, 0x00, 0x00,  /* 52 */
};

static void dac_start_event(struct snd_soc_dapm_widget *w,
    struct snd_kcontrol *kcontrol, int event)
{
    struct snd_soc_codec *codec = w->codec;

    WPRINTK("%s()  event[%x]\n\n", __func__, event);

    switch(event) {
        case SND_SOC_DAPM_POST_PMU:
            snd_soc_update_bits(codec, ES8388_CHIPPOWER, 0x51, 0x00);
            break;
        case SND_SOC_DAPM_PRE_PMD:
            snd_soc_update_bits(codec, ES8388_CHIPPOWER, 0x51, 0x00);
            break;
    }
    return 0;
}

static void adc_start_event(struct snd_soc_dapm_widget *w,
    struct snd_kcontrol *kcontrol, int event)
{
    struct snd_soc_codec *codec = w->codec;

    WPRINTK("%s()  event[%x]\n", __func__, event);

    switch(event) {
        case SND_SOC_DAPM_POST_PMU:
            snd_soc_update_bits(codec, ES8388_CHIPPOWER, 0xA2, 0x00);
            break;
        case SND_SOC_DAPM_PRE_PMD:
            snd_soc_update_bits(codec, ES8388_CHIPPOWER, 0xA2, 0x00);
            break;
    }
    return 0;
}


static const struct snd_kcontrol_new es8388_snd_controls[] = {
    //SOC_DOUBLE_R("Capture Volume", ES8388_LINVOL, ES8388_RINVOL, 0, 63, 0),
    //SOC_DOUBLE_R("Capture ZC Switch", ES8388_LINVOL, ES8388_RINVOL, 6, 1, 0),

    //SOC_DOUBLE_R("Capture Switch", ES8388_LINVOL, ES8388_RINVOL, 7, 1, 1),
    //SOC_SINGLE("Capture Mic Switch", ES8388_ADCIN, 7, 1, 1),

    //SOC_DOUBLE_R("Playback Switch", ES8388_LDAC, ES8388_RDAC,6, 1, 1),
    SOC_DOUBLE_R("Playback Volume", ES8388_LDAC_VOL, ES8388_RDAC_VOL, 0, 192, 0),
};


/*
 * DAPM Controls
 */

/* Channel Input Mixer */
static const char *es8388_line_texts[] = {
	"Line 1", "Line 2", "Differential"};

static const unsigned int es8388_line_values[] = {
	0, 1, 3};

static const struct soc_enum es8388_lline_enum =
    SOC_VALUE_ENUM_SINGLE(ES8388_ADCCONTROL3, 6, 0xC0,
                ARRAY_SIZE(es8388_line_texts),
                es8388_line_texts,
                es8388_line_values);
static const struct snd_kcontrol_new es8388_left_line_controls =
    SOC_DAPM_VALUE_ENUM("Route", es8388_lline_enum);

static const struct soc_enum es8388_rline_enum =
    SOC_VALUE_ENUM_SINGLE(ES8388_ADCCONTROL3, 4, 0x30,
                ARRAY_SIZE(es8388_line_texts),
                es8388_line_texts,
                es8388_line_values);
static const struct snd_kcontrol_new es8388_right_line_controls =
    SOC_DAPM_VALUE_ENUM("Route", es8388_lline_enum);


/* Left Mixer */
static const struct snd_kcontrol_new es8388_left_mixer_controls[] = {
    SOC_DAPM_SINGLE("Left Playback Switch", ES8388_DACCONTROL17, 7, 1, 0),      // 39 
    SOC_DAPM_SINGLE("Left Bypass Switch"  , ES8388_DACCONTROL17, 6, 1, 0),      // 39
};

/* Right Mixer */
static const struct snd_kcontrol_new es8388_right_mixer_controls[] = {
    SOC_DAPM_SINGLE("Right Playback Switch", ES8388_DACCONTROL20, 7, 1, 0),     // 42
    SOC_DAPM_SINGLE("Right Bypass Switch"  , ES8388_DACCONTROL20, 6, 1, 0),     // 42
};


/* Differential Mux */
static const char *es8388_diff_sel[] = {"Line 1", "Line 2"};
static const struct soc_enum left_diffmux =
	SOC_ENUM_SINGLE(ES8388_ADCCONTROL2, 2, 2, es8388_diff_sel);
static const struct snd_kcontrol_new es8388_left_diffmux_controls =
	SOC_DAPM_ENUM("Route", left_diffmux);

static const struct soc_enum right_diffmux =
	SOC_ENUM_SINGLE(ES8388_ADCCONTROL3, 7, 2, es8388_diff_sel);
static const struct snd_kcontrol_new es8388_right_diffmux_controls =
	SOC_DAPM_ENUM("Route", right_diffmux);


/* Mono ADC Mux */
static const char *es8388_mono_mux[] = {"Stereo", "Mono (Left)",
	"Mono (Right)", "NONE"};
static const struct soc_enum monomux =
	SOC_ENUM_SINGLE(ES8388_ADCCONTROL3, 3, 4, es8388_mono_mux);
static const struct snd_kcontrol_new es8388_monomux_controls =
	SOC_DAPM_ENUM("Route", monomux);



static const struct snd_soc_dapm_widget es8388_dapm_widgets[] = {
    // DAC Part
    SND_SOC_DAPM_MIXER("Left Mixer", SND_SOC_NOPM, 0, 0,
        &es8388_left_mixer_controls[0],
        ARRAY_SIZE(es8388_left_mixer_controls)),
    SND_SOC_DAPM_MIXER("Right Mixer", SND_SOC_NOPM, 0, 0,
        &es8388_right_mixer_controls[0],
        ARRAY_SIZE(es8388_right_mixer_controls)),

	SND_SOC_DAPM_MUX("Left Line Mux", SND_SOC_NOPM, 0, 0,
		&es8388_left_line_controls),
    SND_SOC_DAPM_MUX("Right Line Mux", SND_SOC_NOPM, 0, 0,
        &es8388_right_line_controls),

    SND_SOC_DAPM_DAC("Left DAC"  , "Left Playback" , ES8388_DACPOWER, 7, 1),
    SND_SOC_DAPM_DAC("Right DAC" , "Right Playback", ES8388_DACPOWER, 6, 1),
    SND_SOC_DAPM_PGA("Left Out 1" , ES8388_DACPOWER, 5, 0, NULL, 0),
    //SND_SOC_DAPM_PGA("Right Out 1", ES8388_DACPOWER, 4, 0, NULL, 0),
    SND_SOC_DAPM_PGA_E("Right Out 1", ES8388_DACPOWER, 4, 0, NULL, 0, dac_start_event, SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_PRE_PMD),
    //SND_SOC_DAPM_PGA("Left Out 2" , ES8388_DACPOWER, 3, 0, NULL, 0),
    //SND_SOC_DAPM_PGA("Right Out 2", ES8388_DACPOWER, 2, 0, NULL, 0),


	SND_SOC_DAPM_OUTPUT("LOUT1"),
	SND_SOC_DAPM_OUTPUT("ROUT1"),
    SND_SOC_DAPM_OUTPUT("LOUT2"),
    SND_SOC_DAPM_OUTPUT("ROUT2"),
	SND_SOC_DAPM_OUTPUT("VREF"),


#if 1
    //--------------------------------------------
    // ADC Part
    //--------------------------------------------
	SND_SOC_DAPM_MUX("Differential Left Mux", SND_SOC_NOPM, 0, 0,
		&es8388_left_diffmux_controls),
	SND_SOC_DAPM_MUX("Differential Right Mux", SND_SOC_NOPM, 0, 0,
		&es8388_right_diffmux_controls),

	SND_SOC_DAPM_MUX("Left ADC Mux", SND_SOC_NOPM, 0, 0,
		&es8388_monomux_controls),
	SND_SOC_DAPM_MUX("Right ADC Mux", SND_SOC_NOPM, 0, 0,
		&es8388_monomux_controls),


    SND_SOC_DAPM_PGA("Left Analog Input" , ES8388_ADCPOWER, 7, 1, NULL, 0),
    SND_SOC_DAPM_PGA("Right Analog Input", ES8388_ADCPOWER, 6, 1, NULL, 0),
    SND_SOC_DAPM_ADC("Left ADC" , "Left Capture" , ES8388_ADCPOWER, 5, 1),
    SND_SOC_DAPM_ADC("Right ADC", "Right Capture", ES8388_ADCPOWER, 4, 1),

    SND_SOC_DAPM_MICBIAS_E("Mic Bias", ES8388_ADCPOWER, 3, 1, adc_start_event, SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_PRE_PMD),
#endif

    SND_SOC_DAPM_INPUT("MICIN"),
    SND_SOC_DAPM_INPUT("LINPUT1"),
    SND_SOC_DAPM_INPUT("LINPUT2"),
    SND_SOC_DAPM_INPUT("RINPUT1"),
    SND_SOC_DAPM_INPUT("RINPUT2"),
};


static const struct snd_soc_dapm_route intercon[] = {
    /* left mixer */
    {"Left Mixer", "Left Playback Switch"   , "Left DAC"},

    /* right mixer */
    {"Right Mixer", "Right Playback Switch" , "Right DAC"},

    /* left out 1 */
    {"Left Out 1" , NULL, "Left Mixer"},
    {"LOUT1"      , NULL, "Left Out 1"},

    /* right out 1 */
    {"Right Out 1", NULL, "Right Mixer"},
    {"ROUT1"      , NULL, "Right Out 1"},


#if 1
    /* Differential Mux */
    {"Differential Left Mux" , "Line 1", "LINPUT1"},
    {"Differential Right Mux", "Line 1", "RINPUT1"},
    {"Differential Left Mux" , "Line 2", "LINPUT2"},
    {"Differential Right Mux", "Line 2", "RINPUT2"},

    /* Left Line Mux */
    {"Left Line Mux", "Line 1"      , "LINPUT1"},
    {"Left Line Mux", "Line 2"      , "LINPUT2"},
    {"Left Line Mux", "Differential", "Differential Left Mux"},

    /* Right Line Mux */
    {"Right Line Mux", "Line 1"      , "RINPUT1"},
    {"Right Line Mux", "Line 2"      , "RINPUT2"},
    {"Right Line Mux", "Differential", "Differential Right Mux"},

    /* Left ADC Mux */
    {"Left ADC Mux", "Stereo"      , "Left Line Mux"},
//    {"Left ADC Mux", "Mono (Left)" , "Left Line Mux"},

    /* Right ADC Mux */
    {"Right ADC Mux", "Stereo"      , "Right Line Mux"},
//    {"Right ADC Mux", "Mono (Right)", "Right Line Mux"},

    /* ADC */
    {"Left ADC" , NULL, "Left ADC Mux"},
    {"Right ADC", NULL, "Right ADC Mux"},


    {"Left ADC" , NULL, "Mic Bias"},
    {"Right ADC", NULL, "Mic Bias"},

    {"Mic Bias", NULL, "MICIN"},
#endif
};


struct _coeff_div {
	u32 mclk;
	u32 rate;
	u16 fs;
	u8 sr:5;
	u8 single_double:1;
	u8 blckdiv:4;
};

/* codec hifi mclk clock divider coefficients */
static const struct _coeff_div coeff_div[] = {
	/* 8k */
	{12000000,  8000, 1500, 0x1B, 0, 0xa},
    
	/* 11.025k */
	{12000000, 11025, 1088, 0x19, 0, 0xa},
    
    /* 12k */
	{12000000, 12000, 1000, 0x18, 0, 0xa},
    
	/* 16k */
	{12000000, 16000,  750, 0x17, 0, 0x6},
    
	/* 22.05k */
	{12000000, 22050,  544, 0x16, 0, 0x6},
    
    /* 24k */
	{12000000, 24000,  500, 0x15, 0, 0x6},
	
	/* 32k */
	{12000000, 32000,  375, 0x14, 0, 0x6},
    
	/* 44.1k */
	{11289600, 44100,  256, 0x02, 0, 0x4}, /* add for hdmi, zyy 2010.6.19 */
	{12000000, 44100,  272, 0x13, 0, 0x4},
    
	/* 48k */
                  
	{12000000, 48000,  250, 0x12, 0, 0x4},
	{12288000, 48000,  256, 0x02, 0, 0x4}, /* add for hdmi */
    
	/* 88.2k */
	{12000000, 88200,  136, 0x11, 1, 0x2},
    
	/* 96k */
	{12000000, 96000,  125, 0x10, 1, 0x2},
};


static inline int get_coeff(int mclk, int rate)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(coeff_div); i++) {
        if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk) {
            WPRINTK(KERN_ERR "es8388:************************** can get coeff for mclk %d @ rate %d\n",mclk, rate);
            return i;
        }
    }

    printk(KERN_ERR "es8388: could not get coeff for mclk %d @ rate %d\n", mclk, rate);
    return -EINVAL;
}

static int es8388_set_dai_clkdiv(struct snd_soc_dai *codec_dai,
		int div_id, int div)
{
    struct snd_soc_codec *codec = codec_dai->codec;
    u16 reg;

    return 0;
}


static unsigned int es8388_read_reg_cache(struct snd_soc_codec *codec,
				     unsigned int reg)
{
	u16 *cache = codec->reg_cache;
	if (reg >= codec->reg_cache_size)
		return -1;
	return cache[reg];
}

static int es8388_write(struct snd_soc_codec *codec, unsigned int reg,
			     unsigned int value)
{
	u16 *cache = codec->reg_cache;
	u8 data[2];
	int ret;

	WPRINTK("reg=%x,value=%x\n",reg,value);
	data[0] = reg;
	data[1] = value & 0x00ff;

	if (reg < codec->reg_cache_size)
		cache[reg] = value;
	ret = codec->hw_write(codec->control_data, data, 2);
	if (ret == 2)
		return 0;
	if (ret < 0)
		return ret;
	else
		return -EIO;
}

static int es8388_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
    struct snd_soc_codec *codec = codec_dai->codec;
    struct es8388_priv *es8388 = codec->private_data;
    WPRINTK("%s----%d\n",__FUNCTION__,__LINE__);
    switch (freq) {
        case 11289600:
        case 12000000:
        case 12288000:
        case 16934400:
        case 18432000:
            es8388->sysclk = freq;
            return 0;
    }
    return -EINVAL;
}

static int es8388_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
    struct snd_soc_codec *codec = codec_dai->codec;
    u8 iface = 0;
    u8 adciface = 0;
    u8 daciface = 0;
    WPRINTK("%s----%d, fmt[%02x]\n",__FUNCTION__,__LINE__,fmt);

    iface    = snd_soc_read(codec, ES8388_IFACE);
    adciface = snd_soc_read(codec, ES8388_ADC_IFACE);
    daciface = snd_soc_read(codec, ES8388_DAC_IFACE);

    /* set master/slave audio interface */
    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
        case SND_SOC_DAIFMT_CBM_CFM:    // MASTER MODE
        	  WPRINTK("es8388 in master mode");
            iface |= 0x80;
            break;
        case SND_SOC_DAIFMT_CBS_CFS:    // SLAVE MODE
        	  WPRINTK("es8388 in slave mode");
            iface &= 0x7F;
            break;
        default:
            return -EINVAL;
    }

    /* interface format */
    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
        case SND_SOC_DAIFMT_I2S:
            adciface &= 0xFC;
            //daciface &= 0xF9;  //updated by david-everest,5-25           
            daciface &= 0xF9;
            break;
        case SND_SOC_DAIFMT_RIGHT_J:
            break;
        case SND_SOC_DAIFMT_LEFT_J:
            break;
        case SND_SOC_DAIFMT_DSP_A:
            break;
        case SND_SOC_DAIFMT_DSP_B:
            break;
        default:
            return -EINVAL;
    }

    /* clock inversion */
    switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
        case SND_SOC_DAIFMT_NB_NF:
            iface    &= 0xDF;
            adciface &= 0xDF;
            //daciface &= 0xDF;    //UPDATED BY david-everest,5-25        
            daciface &= 0xBF;
            break;
        case SND_SOC_DAIFMT_IB_IF:
            iface    |= 0x20;
            //adciface &= 0xDF;    //UPDATED BY david-everest,5-25
            adciface |= 0x20;
            //daciface &= 0xDF;   //UPDATED BY david-everest,5-25
            daciface |= 0x40;
            break;
        case SND_SOC_DAIFMT_IB_NF:
            iface    |= 0x20;
           // adciface |= 0x40;  //UPDATED BY david-everest,5-25
            adciface &= 0xDF;
            //daciface |= 0x40;  //UPDATED BY david-everest,5-25
            daciface &= 0xBF;
            break;
        case SND_SOC_DAIFMT_NB_IF:
            iface    &= 0xDF;
            adciface |= 0x20;
            //daciface |= 0x20;  //UPDATED BY david-everest,5-25
            daciface |= 0x40;
            break;
        default:
            return -EINVAL;
    }


  WPRINTK("AZ ES8388  ES8388_ADC_IFACE:0X%X  ES8388_IFACE:0X%X",adciface,iface);

    snd_soc_write(codec, ES8388_IFACE    , iface);
    snd_soc_write(codec, ES8388_ADC_IFACE, adciface);
    snd_soc_write(codec, ES8388_DAC_IFACE, daciface);

    return 0;
}

static int es8388_pcm_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params,
		struct snd_soc_dai *dai)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_device *socdev = rtd->socdev;
    struct snd_soc_codec *codec =  socdev->card->codec;
    struct es8388_priv *es8388 = codec->private_data;
	u16 iface;

    WPRINTK("es8388_pcm_hw_params()----%d, sampling rate[%d]\n", __LINE__, params_rate(params));

   if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {

    snd_soc_write(codec,0x0F, 0xf4);     // mute ADC   
    snd_soc_write(codec,0x19, 0xEA);     // DAC unmute
   }
    else
   {  	
    snd_soc_write(codec,0x19, 0xEE);     // DAC MUTE   	
    snd_soc_write(codec,0x0F, 0xf0);     // ADC UNMUTE
    }
	return 0;
}
void codec_set_spk(int on)
{
	if(on){
		gpio_set_value(SPK_CON, SPEAKER_EN_VALUE);
	}else{
		gpio_set_value(SPK_CON, !SPEAKER_EN_VALUE);
	}
	return;
}
EXPORT_SYMBOL(codec_set_spk);
static int es8388_mute(struct snd_soc_dai *dai, int mute)
{
    struct snd_soc_codec *codec = dai->codec;
    unsigned char val = 0;

    WPRINTK("%s----%d, %d\n",__FUNCTION__,__LINE__,mute);

    //val = snd_soc_read(codec, ES8388_DAC_MUTE);
    if (mute){
        snd_soc_write(codec, ES8388_DAC_MUTE, 0xe6);
    }
    else{
        snd_soc_write(codec, ES8388_DAC_MUTE, 0xe2);
    }

#if 0
	if(mute == 0){
		audio_trigger_flag = 1;
		if(HEADPHONE_DET_IN_VALUE != gpio_get_value(HEADPHONE_DET_PIN) ){
		//headphone remove
			#if defined(HEADPHONE_MUTE_PIN)
			if(HEADPHONE_MUTE_PIN != NULL_GPIO){
				gpio_set_value(HEADPHONE_MUTE_PIN, HEADPHONE_MUTE_ACTIVE_LEVEL);//disable headphone
			}
			#endif
			
			#if (defined(CONFIG_ANX7150) || defined(WISKY_HDMI_ANX7150)) && (WISKY_ENABLE_HDMI == 1)
			if(0 == hdmi_status){
				//enable speaker if headphone remove and not in HDMI output
				gpio_set_value(SPK_CON, GPIO_HIGH);				
			}else{
				gpio_set_value(SPK_CON, GPIO_LOW);
			}
			#else
			//enable speaker if headphone remove or not in HDMI output
			gpio_set_value(SPK_CON, GPIO_HIGH);
			#endif
		}
		else{//headphone insert
		
			#if defined(HEADPHONE_MUTE_PIN)
			if(HEADPHONE_MUTE_PIN != NULL_GPIO){
				if(audio_volume != 0){
					gpio_set_value(HEADPHONE_MUTE_PIN, !HEADPHONE_MUTE_ACTIVE_LEVEL);//enable headphone
				}
			}
			#endif
			gpio_set_value(SPK_CON, GPIO_LOW);//disable speaker
		}
	}
	else{
#if defined(HEADPHONE_MUTE_PIN)
		if(HEADPHONE_MUTE_PIN != NULL_GPIO){
			gpio_set_value(HEADPHONE_MUTE_PIN, HEADPHONE_MUTE_ACTIVE_LEVEL);//disable headphone
		}
#endif
		gpio_set_value(SPK_CON, GPIO_LOW);//disable speaker
		audio_trigger_flag = 0;
	}
#endif
	
    return 0;
}


static int es8388_set_bias_level(struct snd_soc_codec *codec,
		enum snd_soc_bias_level level)
{
    int i;
	u16 *cache = codec->reg_cache;

    WPRINTK("AZ %s----%d, %s\n",__FUNCTION__,__LINE__,
        level == 0 ? "BIAS_ON" :
        level == 1 ? "BIAS_PREPARE" :
        level == 2 ? "BIAS_STANDBY" : "BIAS_OFF");

    switch(level)
    {
        case SND_SOC_BIAS_ON:

            break;

        case SND_SOC_BIAS_PREPARE:
           if(codec->bias_level == SND_SOC_BIAS_ON) {

                snd_soc_write(codec, ES8388_ADCPOWER,  0X00);  //0X00 ZYX 0x0C
                snd_soc_write(codec, ES8388_DACPOWER,  0x30);
                snd_soc_write(codec, ES8388_CHIPPOWER, 0x00);
            }
          
            break;

        case SND_SOC_BIAS_STANDBY:

            snd_soc_write(codec, ES8388_ADCPOWER, 0x00);
            snd_soc_write(codec, ES8388_DACPOWER , 0x30);
            snd_soc_write(codec, ES8388_CHIPPOWER , 0x00);               
            break;

        case SND_SOC_BIAS_OFF:

            snd_soc_write(codec, ES8388_ADCPOWER, 0xff);
            snd_soc_write(codec, ES8388_DACPOWER , 0xC0);
            snd_soc_write(codec, ES8388_CHIPPOWER , 0xC3);             
            break;
    }
	codec->bias_level = level;

	return 0;
}


#define ES8388_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
                    SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_44100 | \
                    SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

#define ES8388_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
                    SNDRV_PCM_FMTBIT_S24_LE)

static struct snd_soc_dai_ops es8388_ops = {
    .hw_params    = es8388_pcm_hw_params,
    .set_fmt      = es8388_set_dai_fmt,
    .set_sysclk   = es8388_set_dai_sysclk,
    .digital_mute = es8388_mute,
    //.startup      = es8388_ini,
    .set_clkdiv   = es8388_set_dai_clkdiv,
};


struct snd_soc_dai es8388_dai = {
    .name = "ES8388",
    .playback = {
        .stream_name = "Playback",
        .channels_min = 2,
        .channels_max = 2,
        .rates = ES8388_RATES,
        .formats = ES8388_FORMATS,},
    .capture = {
        .stream_name = "Capture",
        .channels_min = 2,
        .channels_max = 2,
        .rates = ES8388_RATES,
        .formats = ES8388_FORMATS,},
    .ops = &es8388_ops,	
};
EXPORT_SYMBOL_GPL(es8388_dai);

static int es8388_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
	
	gpio_request(SPK_CON, NULL);
	gpio_direction_output(SPK_CON,GPIO_LOW);
	gpio_free(SPK_CON);
	WPRINTK("ES8388 SUSPEND FUNCTION");
   unsigned int i;
  
/*  for(i=0;i<0x1e;i++)
  {
	 snd_soc_write(codec,0x2e, 0x1e-i);  //LOUT/ROUT Volume=-30dB
	 snd_soc_write(codec,0x2f, 0x1e-i);  	
	 mdelay(10);
  }*/
	snd_soc_write(codec,0X19, 0XEE);  //DAC MUTE
	snd_soc_write(codec,0X0F, 0XF4);  //ADC MUTE	
	
 	 snd_soc_write(codec,0x03, 0xff);  //DAC disable,LOUT/ROUT disable
	snd_soc_write(codec,0x04, 0xf0);  //DAC disable,LOUT/ROUT disable

	snd_soc_write(codec,0x02, 0xc3);  //Stop State-machine and DSM

	return 0;
}

static int es8388_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
	int i;
	u8 data[2];
	u16 *cache = codec->reg_cache;

	snd_soc_write(codec,0x03, 0x00);  //DAC disable,LOUT/ROUT disable
	//CodecCommandSet(0x04, 0x00);  //DAC disable,LOUT/ROUT disable
	//mdelay(20);
  	snd_soc_write(codec,0x02, 0x00);  //DAC disable,LOUT/ROUT disable
	snd_soc_write(codec,0x04, 0x30);  //Stop State-machine and DSM
	snd_soc_write(codec,0X19, 0XEE);  //DAC MUTE	
	snd_soc_write(codec,0X0F, 0XF4);  //ADC MUTE	
/*	  for(i=0;i<0x1e;i++)
 	 {
	 snd_soc_write(codec,0x2e, 0X01+i);  //LOUT/ROUT Volume=-6dB
	 snd_soc_write(codec,0x2f, 0X01+i);  	
	 mdelay(10);
  }	*/	
	return 0;
}


/**********************************************************************************************************/
static int es8388_probe(struct platform_device *pdev)
{
    struct snd_soc_device *socdev = platform_get_drvdata(pdev);
    struct snd_soc_codec *codec;
    int ret = 0;

    WPRINTK("%s() \n", __func__);

    if (es8388_codec == NULL) {
        dev_err(&pdev->dev, "Codec device not registered\n");
        return -ENODEV;
    }

    socdev->card->codec = es8388_codec;
    codec = es8388_codec;

    /* register pcms */
    ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
    if (ret < 0) {
        dev_err(codec->dev, "failed to create pcms: %d\n", ret);
        goto pcm_err;
    }


    snd_soc_add_controls(codec, es8388_snd_controls,
                 ARRAY_SIZE(es8388_snd_controls));
    snd_soc_dapm_new_controls(codec, es8388_dapm_widgets,
                  ARRAY_SIZE(es8388_dapm_widgets));
    snd_soc_dapm_add_routes(codec, intercon, ARRAY_SIZE(intercon));
    snd_soc_dapm_new_widgets(codec);


    ret = snd_soc_init_card(socdev);
    if (ret < 0) {
        dev_err(codec->dev, "failed to register card: %d\n", ret);
        goto card_err;
    }

	info("Audio Codec Driver %s", ES8388_VERSION);
    return ret;

card_err:
    snd_soc_free_pcms(socdev);
    snd_soc_dapm_free(socdev);
pcm_err:
    return ret;
}


/* power down chip */
static int es8388_remove(struct platform_device *pdev)
{
    struct snd_soc_device *socdev = platform_get_drvdata(pdev);

    snd_soc_free_pcms(socdev);
    snd_soc_dapm_free(socdev);

    return 0;
}

struct snd_soc_codec_device soc_codec_dev_es8388 = {
	.probe = 	es8388_probe,
	.remove = 	es8388_remove,
	.suspend = 	es8388_suspend,
	.resume =	es8388_resume,
};

EXPORT_SYMBOL_GPL(soc_codec_dev_es8388);



static int es8388_ini(struct snd_soc_codec *codec,
			   enum snd_soc_control_type control)
{
    WPRINTK("es8388 initial");

    snd_soc_write(codec, ES8388_MASTERMODE  , 0x00);    // SLAVE MODE, MCLK not divide
    snd_soc_write(codec, ES8388_CHIPPOWER   , 0xf3);    // Power down: ADC DEM, DAC DSM/DEM, ADC/DAC state machine, ADC/DAC ananlog reference
    snd_soc_write(codec, ES8388_DACCONTROL21, 0x80);    // DACLRC and ADCLRC same, ADC/DAC DLL power up, Enable MCLK input from PAD.

    snd_soc_write(codec, ES8388_CONTROL1   , 0x05); //05    // VMIDSEL (500 kohme divider enabled)
    snd_soc_write(codec, ES8388_CONTROL2 , 0x72);   // 
  
    snd_soc_write(codec, ES8388_DACPOWER   , 0x30);     // DAC R/L Power on, OUT1 enable, OUT2 disable
    snd_soc_write(codec, ES8388_ADCPOWER   , 0x00);     // 
    snd_soc_write(codec, ES8388_ANAVOLMANAG, 0x7C);     // 

    //-----------------------------------------------------------------------------------------------------------------
    snd_soc_write(codec, ES8388_ADCCONTROL1, 0xbb);     // MIC PGA gain: +24dB
    snd_soc_write(codec, ES8388_ADCCONTROL2, 0xf0);     // LINSEL(L-R differential), RINGSEL(L-R differential)
    snd_soc_write(codec, ES8388_ADCCONTROL3, 0x82);     // Input Select: LIN2/RIN2
    snd_soc_write(codec, ES8388_ADCCONTROL4, 0x4C);     // Left data = left ADC, right data = right ADC, 24 bits I2S
    snd_soc_write(codec, ES8388_ADCCONTROL5, 0x02);     // 256fs
    //snd_soc_write(codec, ES8388_ADCCONTROL6, 0x00);     // Disable High pass filter

    snd_soc_write(codec, ES8388_LADC_VOL, 0x00);        // 0dB
    snd_soc_write(codec, ES8388_RADC_VOL, 0x00);        // 0dB

    //snd_soc_write(codec, ES8388_ADCCONTROL10, 0x3A);    // ALC stereo, Max gain(17.5dB), Min gain(0dB)
    snd_soc_write(codec, ES8388_ADCCONTROL10, 0xea);    // ALC stereo, Max gain(17.5dB), Min gain(0dB),updated by david-everest,5-25
    snd_soc_write(codec, ES8388_ADCCONTROL11, 0xA0);    // ALCLVL(-1.5dB), ALCHLD(0ms)
    snd_soc_write(codec, ES8388_ADCCONTROL12, 0x05);    // ALCDCY(1.64ms/363us), ALCATK(1664us/363.2us)
    snd_soc_write(codec, ES8388_ADCCONTROL13, 0x06);    // ALCMODE(ALC mode), ALCZC(disable), TIME_OUT(disable), WIN_SIZE(96 samples)
    snd_soc_write(codec, ES8388_ADCCONTROL14, 0xc3);    // NGTH(XXX), NGG(mute ADC output), NGAT(enable)


    //----------------------------------------------------------------------------------------------------------------
    snd_soc_write(codec, ES8388_DACCONTROL1, 0x18);     // I2S 16bits 
    snd_soc_write(codec, ES8388_DACCONTROL2, 0x02);     // 256fs

    snd_soc_write(codec, ES8388_LDAC_VOL, 0x00);    // left DAC volume
    snd_soc_write(codec, ES8388_RDAC_VOL, 0x00);    // right DAC volume

    snd_soc_write(codec, ES8388_DACCONTROL3, 0xE2);     // DAC unmute

    snd_soc_write(codec, ES8388_DACCONTROL17, 0xb8);    // left DAC to left mixer enable, 
    snd_soc_write(codec, ES8388_DACCONTROL18, 0x38);    // ???
    snd_soc_write(codec, ES8388_DACCONTROL19, 0x38);    // ???
    snd_soc_write(codec, ES8388_DACCONTROL20, 0xb8);    // right DAC to right mixer enable,
#if 0
   snd_soc_write(codec, 0x1E,0x02);  //low and high voice adjust  3db
   snd_soc_write(codec, 0x1F,0x33);
   snd_soc_write(codec, 0x20,0x25);
   snd_soc_write(codec, 0x21,0x5C);
   snd_soc_write(codec, 0x22,0x23);
   snd_soc_write(codec, 0x23,0x24);
   snd_soc_write(codec, 0x24,0x6B);
   snd_soc_write(codec, 0x25,0x6E);
#else
snd_soc_write(codec,0x1E,0x02);   // 6db
snd_soc_write(codec,0x1F,0x33);
snd_soc_write(codec,0x20,0x25);
snd_soc_write(codec,0x21,0x5C);
snd_soc_write(codec,0x22,0x20);
snd_soc_write(codec,0x23,0x2A);
snd_soc_write(codec,0x24,0x5F);
snd_soc_write(codec,0x25,0x4E);
/*
snd_soc_write(codec,0x1E,0x01);
snd_soc_write(codec,0x1F,0x21);
snd_soc_write(codec,0x20,0xD4);
snd_soc_write(codec,0x21,0xD9);
snd_soc_write(codec,0x22,0x20);
snd_soc_write(codec,0x23,0x2A);
snd_soc_write(codec,0x24,0x5F);
snd_soc_write(codec,0x25,0x6D);
  */
#endif
  
    snd_soc_write(codec, ES8388_CHIPPOWER, 0x00);   // ALL Block POWER ON
    //snd_soc_write(codec, ES8388_CONTROL2 , 0x72);   // updated by david-everest,5-25
    //mdelay(100);

    snd_soc_write(codec, ES8388_LOUT1_VOL, 0x1d);   // 
    snd_soc_write(codec, ES8388_ROUT1_VOL, 0x1d);   // 
    snd_soc_write(codec, ES8388_LOUT2_VOL, 0x00);   // Disable LOUT2
    snd_soc_write(codec, ES8388_ROUT2_VOL, 0x00);   // Disable ROUT2


	return 0;

}


/*******************************************************************************************************/
static int es8388_register(struct es8388_priv *es8388,
			   enum snd_soc_control_type control)
{
	int ret;
	struct snd_soc_codec *codec = &es8388->codec;

    WPRINTK("es8388 initial");
    
	if (es8388_codec) {
		dev_err(codec->dev, "Another ES8388 is registered\n");
		ret = -EINVAL;
		goto err;
	}

//	gpio_direction_output(SPK_CON, GPIO_LOW);
//	audio_trigger_flag = 0;
	
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	codec->private_data = es8388;
	codec->name = "ES8388";
	codec->owner = THIS_MODULE;
    codec->read  = es8388_read_reg_cache;
    codec->write = es8388_write;
    codec->hw_write = (hw_write_t)i2c_master_send;
	codec->bias_level = SND_SOC_BIAS_OFF;
	codec->set_bias_level = es8388_set_bias_level;
	codec->dai = &es8388_dai;
	codec->num_dai = 1;
	codec->reg_cache_size = ES8388_CACHEREGNUM;
	codec->reg_cache = &es8388->reg_cache;

	memcpy(codec->reg_cache, es8388_reg, sizeof(es8388_reg));

	es8388_dai.dev = codec->dev;
 
	es8388_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

    snd_soc_write(codec, ES8388_MASTERMODE  , 0x00);    // SLAVE MODE, MCLK not divide
    snd_soc_write(codec, ES8388_CHIPPOWER   , 0xf3);    // Power down: ADC DEM, DAC DSM/DEM, ADC/DAC state machine, ADC/DAC ananlog reference
    snd_soc_write(codec, ES8388_DACCONTROL21, 0x80);    // DACLRC and ADCLRC same, ADC/DAC DLL power up, Enable MCLK input from PAD.

    snd_soc_write(codec, ES8388_CONTROL1   , 0x05);     // VMIDSEL (500 kohme divider enabled)
    snd_soc_write(codec, ES8388_CONTROL2 , 0x72);   // 
  
    snd_soc_write(codec, ES8388_DACPOWER   , 0x30);     // DAC R/L Power on, OUT1 enable, OUT2 disable
    snd_soc_write(codec, ES8388_ADCPOWER   , 0x00);     // 
    snd_soc_write(codec, ES8388_ANAVOLMANAG, 0x7f);     // 0x7c


    //-----------------------------------------------------------------------------------------------------------------
    snd_soc_write(codec, ES8388_ADCCONTROL1, 0xbb);     // MIC PGA gain: +24dB
    snd_soc_write(codec, ES8388_ADCCONTROL2, 0xf0);     // LINSEL(L-R differential), RINGSEL(L-R differential)
    snd_soc_write(codec, ES8388_ADCCONTROL3, 0x82);     // Input Select: LIN2/RIN2
    snd_soc_write(codec, ES8388_ADCCONTROL4, 0x4C);     // Left data = left ADC, right data = right ADC, 24 bits I2S
    snd_soc_write(codec, ES8388_ADCCONTROL5, 0x02);     // 256fs
    //snd_soc_write(codec, ES8388_ADCCONTROL6, 0x00);     // Disable High pass filter

    snd_soc_write(codec, ES8388_LADC_VOL, 0x00);        // 0dB
    snd_soc_write(codec, ES8388_RADC_VOL, 0x00);        // 0dB

    //snd_soc_write(codec, ES8388_ADCCONTROL10, 0x3A);    // ALC stereo, Max gain(17.5dB), Min gain(0dB)
    snd_soc_write(codec, ES8388_ADCCONTROL10, 0xe2);    // ALC stereo, Max gain(17.5dB), Min gain(0dB),updated by david-everest,5-25
    snd_soc_write(codec, ES8388_ADCCONTROL11, 0xA0);    // ALCLVL(-1.5dB), ALCHLD(0ms)
    snd_soc_write(codec, ES8388_ADCCONTROL12, 0x05);    // ALCDCY(1.64ms/363us), ALCATK(1664us/363.2us)
    snd_soc_write(codec, ES8388_ADCCONTROL13, 0x06);    // ALCMODE(ALC mode), ALCZC(disable), TIME_OUT(disable), WIN_SIZE(96 samples)
    snd_soc_write(codec, ES8388_ADCCONTROL14, 0xd3);    // NGTH(XXX), NGG(mute ADC output), NGAT(enable)


    //----------------------------------------------------------------------------------------------------------------
    snd_soc_write(codec, ES8388_DACCONTROL1, 0x18);     // I2S 16bits 
    snd_soc_write(codec, ES8388_DACCONTROL2, 0x02);     // 256fs

    snd_soc_write(codec, ES8388_LDAC_VOL, 0x00);    // left DAC volume
    snd_soc_write(codec, ES8388_RDAC_VOL, 0x00);    // right DAC volume

    snd_soc_write(codec, ES8388_DACCONTROL3, 0xE2);     // DAC unmute

    snd_soc_write(codec, ES8388_DACCONTROL17, 0xb8);    // left DAC to left mixer enable, 
    snd_soc_write(codec, ES8388_DACCONTROL18, 0x38);    // ???
    snd_soc_write(codec, ES8388_DACCONTROL19, 0x38);    // ???
    snd_soc_write(codec, ES8388_DACCONTROL20, 0xb8);    // right DAC to right mixer enable,

    snd_soc_write(codec, ES8388_CHIPPOWER, 0x00);   // ALL Block POWER ON
    //snd_soc_write(codec, ES8388_CONTROL2 , 0x72);   // updated by david-everest,5-25
    //mdelay(100);

    snd_soc_write(codec, ES8388_LOUT1_VOL, 0x1d);   // 
    snd_soc_write(codec, ES8388_ROUT1_VOL, 0x1d);   // 
    snd_soc_write(codec, ES8388_LOUT2_VOL, 0x00);   // Disable LOUT2
    snd_soc_write(codec, ES8388_ROUT2_VOL, 0x00);   // Disable ROUT2


	es8388_codec = codec;

	ret = snd_soc_register_codec(codec);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register codec: %d\n", ret);
		goto err;
	}

	ret = snd_soc_register_dai(&es8388_dai);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register DAI: %d\n", ret);
		snd_soc_unregister_codec(codec);
		goto err_codec;
	}

	return 0;

err_codec:
	snd_soc_unregister_codec(codec);
err:
	kfree(es8388);
	return ret;
	
}

static void es8388_unregister(struct es8388_priv *es8388)
{
    WPRINTK("%s() \n", __func__);

	es8388_set_bias_level(&es8388->codec, SND_SOC_BIAS_OFF);
	snd_soc_unregister_dai(&es8388_dai);
	snd_soc_unregister_codec(&es8388->codec);
	kfree(es8388);
	es8388_codec = NULL;
}

#if defined (CONFIG_I2C) || defined (CONFIG_I2C_MODULE)

static int es8388_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct es8388_priv *es8388;
	struct snd_soc_codec *codec;

    WPRINTK("%s() \n", __func__);

	es8388 = kzalloc(sizeof(struct es8388_priv), GFP_KERNEL);
	if (es8388 == NULL)
		return -ENOMEM;

	codec = &es8388->codec;

	i2c_set_clientdata(i2c, es8388);
	codec->control_data = i2c;

	codec->dev = &i2c->dev;

	return es8388_register(es8388, SND_SOC_I2C);
}

static int es8388_i2c_remove(struct i2c_client *client)
{
    struct es8388_priv *es8388 = i2c_get_clientdata(client);

    WPRINTK("%s() \n", __func__);

    es8388_unregister(es8388);
    return 0;
}

#ifdef CONFIG_PM
static int es8388_i2c_suspend(struct i2c_client *i2c, pm_message_t msg)
{
	return snd_soc_suspend_device(&i2c->dev);
}

static int es8388_i2c_resume(struct i2c_client *i2c)
{
	return snd_soc_resume_device(&i2c->dev);
}
#else
#define es8388_i2c_suspend NULL
#define es8388_i2c_resume NULL
#endif

static const struct i2c_device_id es8388_i2c_id[] = {
	{ "es8388", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, es8388_i2c_id);

static struct i2c_driver es8388_i2c_driver = {
	.driver = {
		.name = "es8388 I2C Codec",
		.owner = THIS_MODULE,
	},
	.probe    = es8388_i2c_probe,
	.remove   = es8388_i2c_remove,
	.suspend  = es8388_i2c_suspend,
	.resume   = es8388_i2c_resume,
	.id_table = es8388_i2c_id,
};


static int __init es8388_modinit(void)
{
	int ret;

    WPRINTK("%s() \n", __func__);
	ret = i2c_add_driver(&es8388_i2c_driver);
	if (ret != 0) {
		printk(KERN_ERR "Failed to register ES8388 I2C driver: %d\n",
		       ret);
	}

	return 0;
}
module_init(es8388_modinit);

static void __exit es8388_exit(void)
{
    WPRINTK("%s() \n", __func__);
	i2c_del_driver(&es8388_i2c_driver);
}
module_exit(es8388_exit);
#endif


MODULE_DESCRIPTION("ASoC es8388 driver");
MODULE_AUTHOR("Liam Girdwood");
MODULE_LICENSE("GPL");
