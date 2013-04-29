/* wisky/lcd/wisky_lcd_ej080na048.c
 *
 * Copyright (C) 2011 Wisky Ltd
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
 * V001:20110509 lxh
 *	1.Create for EJ080NA048 LCD module driver.
 *		�������ӣ�INNOLUX��EJ080NA048 LCD Module��
 ��		LCD size��8.0Inch��Diagonal��
 ��		Resolution��1024X768
 */

#include <linux/kernel.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/rk_fb.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <mach/board.h>
#include "../../drivers/video/rk29_fb.h"
//#include "../../drivers/video/display/screen/screen.h"
#include "../../drivers/video/rockchip/hdmi/rk_hdmi.h"
//#include "screen.h"

#ifdef CONFIG_RK610_LVDS
#include "../../drivers/video/display/transmitter/rk610_lcd.h"
#endif
/*type*/
/* Base */
#ifdef CONFIG_RK610_LVDS
#define OUT_TYPE	    	SCREEN_LVDS
#define OUT_FORMAT      	LVDS_8BIT_3
#else
#define OUT_TYPE	    SCREEN_RGB
#endif

#define OUT_FACE	    OUT_D888_P666


#define OUT_CLK	          67000000
#define LCDC_ACLK        300000000           //29 lcdc axi DMA Ƶ��

/* Timing */
#define H_PW			10
#define H_BP			64
#define H_VD			WISKY_LCD_WIDTH
#define H_FP			18

#define V_PW			80
#define V_BP			8
#define V_VD			WISKY_LCD_HEIGHT
#define V_FP			6

//����LCD ��������д��Ч����
#define LCD_ACTIVE_WIDTH_MM          94
#define LCD_ACTIVE_HEIGHT_MM         150

/* Other */
#define DCLK_POL		1
#define SWAP_RB		0

int dsp_lut[256] ={
		0x00000000, 0x00010101, 0x00020202, 0x00030303, 0x00040404, 0x00050505, 0x00060606, 0x00070707, 
		0x00080808, 0x00090909, 0x000a0a0a, 0x000b0b0b, 0x000c0c0c, 0x000d0d0d, 0x000e0e0e, 0x000f0f0f, 
		0x00101010, 0x00111111, 0x00121212, 0x00131313, 0x00141414, 0x00151515, 0x00161616, 0x00171717, 
		0x00181818, 0x00191919, 0x001a1a1a, 0x001b1b1b, 0x001c1c1c, 0x001d1d1d, 0x001e1e1e, 0x001f1f1f, 
		0x00202020, 0x00212121, 0x00222222, 0x00232323, 0x00242424, 0x00252525, 0x00262626, 0x00272727, 
		0x00282828, 0x00292929, 0x002a2a2a, 0x002b2b2b, 0x002c2c2c, 0x002d2d2d, 0x002e2e2e, 0x002f2f2f, 
		0x00303030, 0x00313131, 0x00323232, 0x00333333, 0x00343434, 0x00353535, 0x00363636, 0x00373737, 
		0x00383838, 0x00393939, 0x003a3a3a, 0x003b3b3b, 0x003c3c3c, 0x003d3d3d, 0x003e3e3e, 0x003f3f3f, 
		0x00404040, 0x00414141, 0x00424242, 0x00434343, 0x00444444, 0x00454545, 0x00464646, 0x00474747, 
		0x00484848, 0x00494949, 0x004a4a4a, 0x004b4b4b, 0x004c4c4c, 0x004d4d4d, 0x004e4e4e, 0x004f4f4f, 
		0x00505050, 0x00515151, 0x00525252, 0x00535353, 0x00545454, 0x00555555, 0x00565656, 0x00575757, 
		0x00585858, 0x00595959, 0x005a5a5a, 0x005b5b5b, 0x005c5c5c, 0x005d5d5d, 0x005e5e5e, 0x005f5f5f, 
		0x00606060, 0x00616161, 0x00626262, 0x00636363, 0x00646464, 0x00656565, 0x00666666, 0x00676767, 
		0x00686868, 0x00696969, 0x006a6a6a, 0x006b6b6b, 0x006c6c6c, 0x006d6d6d, 0x006e6e6e, 0x006f6f6f, 
		0x00707070, 0x00717171, 0x00727272, 0x00737373, 0x00747474, 0x00757575, 0x00767676, 0x00777777, 
		0x00787878, 0x00797979, 0x007a7a7a, 0x007b7b7b, 0x007c7c7c, 0x007d7d7d, 0x007e7e7e, 0x007f7f7f, 
		0x00808080, 0x00818181, 0x00828282, 0x00838383, 0x00848484, 0x00858585, 0x00868686, 0x00878787, 
		0x00888888, 0x00898989, 0x008a8a8a, 0x008b8b8b, 0x008c8c8c, 0x008d8d8d, 0x008e8e8e, 0x008f8f8f, 
		0x00909090, 0x00919191, 0x00929292, 0x00939393, 0x00949494, 0x00959595, 0x00969696, 0x00979797, 
		0x00989898, 0x00999999, 0x009a9a9a, 0x009b9b9b, 0x009c9c9c, 0x009d9d9d, 0x009e9e9e, 0x009f9f9f, 
		0x00a0a0a0, 0x00a1a1a1, 0x00a2a2a2, 0x00a3a3a3, 0x00a4a4a4, 0x00a5a5a5, 0x00a6a6a6, 0x00a7a7a7, 
		0x00a8a8a8, 0x00a9a9a9, 0x00aaaaaa, 0x00ababab, 0x00acacac, 0x00adadad, 0x00aeaeae, 0x00afafaf, 
		0x00b0b0b0, 0x00b1b1b1, 0x00b2b2b2, 0x00b3b3b3, 0x00b4b4b4, 0x00b5b5b5, 0x00b6b6b6, 0x00b7b7b7, 
		0x00b8b8b8, 0x00b9b9b9, 0x00bababa, 0x00bbbbbb, 0x00bcbcbc, 0x00bdbdbd, 0x00bebebe, 0x00bfbfbf, 
		0x00c0c0c0, 0x00c1c1c1, 0x00c2c2c2, 0x00c3c3c3, 0x00c4c4c4, 0x00c5c5c5, 0x00c6c6c6, 0x00c7c7c7, 
		0x00c8c8c8, 0x00c9c9c9, 0x00cacaca, 0x00cbcbcb, 0x00cccccc, 0x00cdcdcd, 0x00cecece, 0x00cfcfcf, 
		0x00d0d0d0, 0x00d1d1d1, 0x00d2d2d2, 0x00d3d3d3, 0x00d4d4d4, 0x00d5d5d5, 0x00d6d6d6, 0x00d7d7d7, 
		0x00d8d8d8, 0x00d9d9d9, 0x00dadada, 0x00dbdbdb, 0x00dcdcdc, 0x00dddddd, 0x00dedede, 0x00dfdfdf, 
		0x00e0e0e0, 0x00e1e1e1, 0x00e2e2e2, 0x00e3e3e3, 0x00e4e4e4, 0x00e5e5e5, 0x00e6e6e6, 0x00e7e7e7, 
		0x00e8e8e8, 0x00e9e9e9, 0x00eaeaea, 0x00ebebeb, 0x00ececec, 0x00ededed, 0x00eeeeee, 0x00efefef, 
		0x00f0f0f0, 0x00f1f1f1, 0x00f2f2f2, 0x00f3f3f3, 0x00f4f4f4, 0x00f5f5f5, 0x00f6f6f6, 0x00f7f7f7, 
		0x00f8f8f8, 0x00f9f9f9, 0x00fafafa, 0x00fbfbfb, 0x00fcfcfc, 0x00fdfdfd, 0x00fefefe, 0x00ffffff, 
};

#if  defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF)&& defined(CONFIG_RK610_LVDS)

/* scaler Timing    */
//1920*1080*60

#define S_OUT_CLK		SCALE_RATE(148500000,    64968750)
#define S_H_PW			10
#define S_H_BP			10
#define S_H_VD			800
#define S_H_FP			27

#define S_V_PW			10
#define S_V_BP			10
#define S_V_VD			1200
#define S_V_FP			50

#define S_H_ST			0	
#define S_V_ST		    22 

//1920*1080*50
#define S1_OUT_CLK		SCALE_RATE(148500000,    60328125)
#define S1_H_PW			10
#define S1_H_BP			10
#define S1_H_VD			800
#define S1_H_FP			84

#define S1_V_PW			10
#define S1_V_BP			10
#define S1_V_VD			1200
#define S1_V_FP			79

#define S1_H_ST		    0	
#define S1_V_ST			25

//1280*720*60
#define S2_OUT_CLK		SCALE_RATE(74250000,69918747)
#define S2_H_PW			10
#define S2_H_BP			10
#define S2_H_VD			800
#define S2_H_FP			54

#define S2_V_PW			10
#define S2_V_BP			10
#define S2_V_VD			1200
#define S2_V_FP			79

#define S2_H_ST			873
#define S2_V_ST			13

//1280*720*50

#define S3_OUT_CLK		SCALE_RATE(74250000,60018749)
#define S3_H_PW			10
#define S3_H_BP			10
#define S3_H_VD			800
#define S3_H_FP			84

#define S3_V_PW			10
#define S3_V_BP			10
#define S3_V_VD			1200
#define S3_V_FP			79

#define S3_H_ST			548
#define S3_V_ST			11

//720*576*50
#define S4_OUT_CLK		SCALE_RATE(27000000,60000000) 
#define S4_H_PW			10
#define S4_H_BP			10
#define S4_H_VD			800
#define S4_H_FP			38

#define S4_V_PW			10
#define S4_V_BP			10
#define S4_V_VD			1200
#define S4_V_FP			79

#define S4_H_ST			768
#define S4_V_ST			40

//720*480*60
#define S5_OUT_CLK		SCALE_RATE(27000000,69954544)
#define S5_H_PW			10
#define S5_H_BP			10
#define S5_H_VD			800
#define S5_H_FP			14

#define S5_V_PW			10
#define S5_V_BP			10
#define S5_V_VD			1200
#define S5_V_FP			79

#define S5_H_ST			473
#define S5_V_ST			32

#define S_DCLK_POL       1

static int set_scaler_info(struct rk29fb_screen *screen, u8 hdmi_resolution)
{
    screen->s_clk_inv = S_DCLK_POL;
    screen->s_den_inv = 0;
    screen->s_hv_sync_inv = 0;
    switch(hdmi_resolution){
    case HDMI_1920x1080p_60Hz:
                /* Scaler Timing    */
            screen->hdmi_resolution = hdmi_resolution;
	        screen->s_pixclock = S_OUT_CLK;
	        screen->s_hsync_len = S_H_PW;
	        screen->s_left_margin = S_H_BP;
	        screen->s_right_margin = S_H_FP;
	        screen->s_hsync_len = S_H_PW;
	        screen->s_upper_margin = S_V_BP;
	        screen->s_lower_margin = S_V_FP;
	        screen->s_vsync_len = S_V_PW;
	        screen->s_hsync_st = S_H_ST;
	        screen->s_vsync_st = S_V_ST;
	        break;
	case HDMI_1920x1080p_50Hz:
                /* Scaler Timing    */
            screen->hdmi_resolution = hdmi_resolution;
	        screen->s_pixclock = S1_OUT_CLK;
	        screen->s_hsync_len = S1_H_PW;
	        screen->s_left_margin = S1_H_BP;
	        screen->s_right_margin = S1_H_FP;
	        screen->s_hsync_len = S1_H_PW;
	        screen->s_upper_margin = S1_V_BP;
	        screen->s_lower_margin = S1_V_FP;
	        screen->s_vsync_len = S1_V_PW;
	        screen->s_hsync_st = S1_H_ST;
	        screen->s_vsync_st = S1_V_ST;
	        break;
	case HDMI_1280x720p_60Hz:
                /* Scaler Timing    */
            screen->hdmi_resolution = hdmi_resolution;
	        screen->s_pixclock = S2_OUT_CLK;
	        screen->s_hsync_len = S2_H_PW;
	        screen->s_left_margin = S2_H_BP;
	        screen->s_right_margin = S2_H_FP;
	        screen->s_hsync_len = S2_H_PW;
	        screen->s_upper_margin = S2_V_BP;
	        screen->s_lower_margin = S2_V_FP;
	        screen->s_vsync_len = S2_V_PW;
	        screen->s_hsync_st = S2_H_ST;
	        screen->s_vsync_st = S2_V_ST;
	        break;
    case HDMI_1280x720p_50Hz:
                /* Scaler Timing    */
            screen->hdmi_resolution = hdmi_resolution;
	        screen->s_pixclock = S3_OUT_CLK;
	        screen->s_hsync_len = S3_H_PW;
	        screen->s_left_margin = S3_H_BP;
	        screen->s_right_margin = S3_H_FP;
	        screen->s_hsync_len = S3_H_PW;
	        screen->s_upper_margin = S3_V_BP;
	        screen->s_lower_margin = S3_V_FP;
	        screen->s_vsync_len = S3_V_PW;
	        screen->s_hsync_st = S3_H_ST;
	        screen->s_vsync_st = S3_V_ST;
	        break;
    case HDMI_720x576p_50Hz_4_3:
    case HDMI_720x576p_50Hz_16_9:
                /* Scaler Timing    */
            screen->hdmi_resolution = hdmi_resolution;
	        screen->s_pixclock = S4_OUT_CLK;
	        screen->s_hsync_len = S4_H_PW;
	        screen->s_left_margin = S4_H_BP;
	        screen->s_right_margin = S4_H_FP;
	        screen->s_hsync_len = S4_H_PW;
	        screen->s_upper_margin = S4_V_BP;
	        screen->s_lower_margin = S4_V_FP;
	        screen->s_vsync_len = S4_V_PW;
	        screen->s_hsync_st = S4_H_ST;
	        screen->s_vsync_st = S4_V_ST;
	        break;
    case HDMI_720x480p_60Hz_16_9:
    case HDMI_720x480p_60Hz_4_3:
                /* Scaler Timing    */
            screen->hdmi_resolution = hdmi_resolution;
	        screen->s_pixclock = S5_OUT_CLK;
	        screen->s_hsync_len = S5_H_PW;
	        screen->s_left_margin = S5_H_BP;
	        screen->s_right_margin = S5_H_FP;
	        screen->s_hsync_len = S5_H_PW;
	        screen->s_upper_margin = S5_V_BP;
	        screen->s_lower_margin = S5_V_FP;
	        screen->s_vsync_len = S5_V_PW;
	        screen->s_hsync_st = S5_H_ST;
	        screen->s_vsync_st = S5_V_ST;
	        break;
    default :
            printk("%s lcd not support dual display at this hdmi resolution %d \n",__func__,hdmi_resolution);
            return -1;
	        break;
	}
	
	return 0;
}
#else
#define set_scaler_info  NULL
#endif
void set_lcd_info(struct rk29fb_screen *screen, struct rk29lcd_info *lcd_info)
{
	/* screen type & face */
	screen->face = OUT_FACE;
	screen->type = OUT_TYPE;
#ifdef CONFIG_RK610_LVDS
	screen->hw_format = OUT_FORMAT;
#endif

	/* Screen size */
	screen->x_res = H_VD;
	screen->y_res = V_VD;

	screen->width = LCD_ACTIVE_WIDTH_MM;
	screen->height = LCD_ACTIVE_HEIGHT_MM;

	/* Timing */
	screen->lcdc_aclk = LCDC_ACLK;
	screen->pixclock = OUT_CLK;
	screen->left_margin = H_BP;
	screen->right_margin = H_FP;
	screen->hsync_len = H_PW;
	screen->upper_margin = V_BP;
	screen->lower_margin = V_FP;
	screen->vsync_len = V_PW;

	/* Pin polarity */
	screen->pin_hsync = 0;
	screen->pin_vsync = 0;
	screen->pin_den = 0;
	screen->pin_dclk = DCLK_POL;

	/* Swap rule */
	screen->swap_rb = SWAP_RB;
	screen->swap_rg = 0;
	screen->swap_gb = 0;
	screen->swap_delta = 0;
	screen->swap_dumy = 0;

	/* Operation function*/
	screen->init = NULL;
	screen->standby = NULL;
	//screen->dsp_lut = dsp_lut;
	screen->sscreen_get = set_scaler_info;
#ifdef CONFIG_RK610_LVDS
    screen->sscreen_set = rk610_lcd_scaler_set_param;
#endif
}

//#define LCD_CS_PIN         			LCD_LVDS_PDN_PIN
//#define LCD_CS_MUX_NAME			LCD_LVDS_PDN_MUX_NAME
//#define LCD_CS_MUX_MODE			LCD_LVDS_PDN_MUX_MODE
//#define LCD_CS_VALUE       			LCD_LVDS_PDN_VALUE

#define RK_LCD_EN_PIN         		LCD_PWR_EN_PIN
#define RK_LCD_EN_MUX_NAME		LCD_PWR_EN_MUX_NAME
#define RK_LCD_EN_MUX_MODE		LCD_PWR_EN_MUX_MODE
#define RK_LCD_EN_VALUE			LCD_PWR_EN_VALUE

#define RK_LCD_RESET_PIN				LCD_RESET_PIN
#define RK_LCD_RESET_MUX_NAME		LCD_RESET_MUX_NAME
#define RK_LCD_RESET_MUX_MODE		LCD_RESET_MUX_MODE
#define RK_LCD_RESET_VALUE			LCD_RESET_VALUE

#define RK_LCD_STANDBY_PIN			LCD_STANDBY_PIN
#define RK_LCD_STANDBY_MUX_NAME	LCD_STANDBY_MUX_NAME
#define RK_LCD_STANDBY_MUX_MODE	LCD_STANDBY_MUX_MODE
#define RK_LCD_STANDBY_VALUE		GPIO_HIGH


int lcd_io_init(void)
{
	int ret = 0;

	if(RK_LCD_STANDBY_PIN != INVALID_GPIO){
		rk30_mux_api_set(RK_LCD_STANDBY_MUX_NAME, RK_LCD_STANDBY_MUX_MODE);
		ret = gpio_request(RK_LCD_STANDBY_PIN, NULL);
		if(ret !=0){
			printk(KERN_ERR "request lcd standby pin failed!\n");
		}else{
			gpio_direction_output(RK_LCD_STANDBY_PIN, GPIO_HIGH);
			gpio_set_value(RK_LCD_STANDBY_PIN, RK_LCD_STANDBY_VALUE);
			mdelay(5);
		}
	}

	if(RK_LCD_EN_PIN != INVALID_GPIO){
		rk30_mux_api_set(RK_LCD_EN_MUX_NAME, RK_LCD_EN_MUX_MODE);
		ret = gpio_request(RK_LCD_EN_PIN, NULL);
		if (ret != 0){
			printk(KERN_ERR "request lcd en2 pin fail!\n");
		}
		else{
			gpio_direction_output(RK_LCD_EN_PIN, 1);
			gpio_set_value(RK_LCD_EN_PIN, RK_LCD_EN_VALUE);
		}
	}

	if(RK_LCD_RESET_PIN != INVALID_GPIO){
		rk30_mux_api_set(RK_LCD_RESET_MUX_NAME, RK_LCD_RESET_MUX_MODE);
		ret = gpio_request(RK_LCD_RESET_PIN, NULL);
		if(ret !=0){
			printk(KERN_ERR "request lcd reset pin failed!\n");
		}else{
			mdelay(50);
			gpio_pull_updown(RK_LCD_RESET_PIN, GPIOPullUp);
			gpio_direction_output(RK_LCD_RESET_PIN, GPIO_HIGH);
			gpio_set_value(RK_LCD_RESET_PIN, RK_LCD_RESET_VALUE);
		}
	}

	if(LCD_CS_PIN != INVALID_GPIO){
		rk30_mux_api_set(LCD_CS_MUX_NAME, LCD_CS_MUX_MODE);
		ret = gpio_request(LCD_CS_PIN, NULL);
		if (ret != 0){
			printk(KERN_ERR "request lcd en pin fail!\n");
		}
		else{
			gpio_direction_output(LCD_CS_PIN, 1);
			gpio_set_value(LCD_CS_PIN, LCD_CS_VALUE);
		}
	}

	return 0;
}

int lcd_io_disable(void)
{
	if(LCD_CS_PIN != INVALID_GPIO){
		gpio_set_value(LCD_CS_PIN, !LCD_CS_VALUE);
	}

	if(RK_LCD_STANDBY_PIN != INVALID_GPIO){
		gpio_set_value(RK_LCD_STANDBY_PIN, !RK_LCD_STANDBY_VALUE);
	}
	if(RK_LCD_RESET_PIN != INVALID_GPIO){
		gpio_set_value(RK_LCD_RESET_PIN, !RK_LCD_RESET_VALUE);
	}
	mdelay(20);
	if(RK_LCD_EN_PIN != INVALID_GPIO){
		gpio_set_value(RK_LCD_EN_PIN, !RK_LCD_EN_VALUE);
	}

	return 0;
}

int lcd_io_enable(void)
{
	if(RK_LCD_STANDBY_PIN != INVALID_GPIO){
		gpio_set_value(RK_LCD_STANDBY_PIN, RK_LCD_STANDBY_VALUE);
		mdelay(5);
	}	
	if(RK_LCD_EN_PIN != INVALID_GPIO){
		gpio_set_value(RK_LCD_EN_PIN, RK_LCD_EN_VALUE);
	}
	if(RK_LCD_RESET_PIN != INVALID_GPIO){
		mdelay(50);
		gpio_set_value(RK_LCD_RESET_PIN, RK_LCD_RESET_VALUE);
	}
	if(LCD_CS_PIN != INVALID_GPIO){
		gpio_set_value(LCD_CS_PIN, LCD_CS_VALUE);
	}

	return 0;
}
//wisky[e]
