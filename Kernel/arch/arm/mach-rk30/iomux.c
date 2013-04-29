/*
 * arch/arm/mach-rk29/iomux.c
 *
 *Copyright (C) 2010 ROCKCHIP, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/spinlock.h>

#include <mach/io.h>  
#include <mach/iomux.h>

//#define IOMUX_DBG

static struct mux_config rk30_muxs[] = {
/*
 *	 description				mux  mode   mux	  mux  
 *						reg  offset inter mode
 */ 
//GPIO0A
MUX_CFG(GPIO0A7_I2S8CHSDI_NAME, 			GPIO0A,   14,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0A6_HOSTDRVVBUS_NAME,			GPIO0A,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0A5_OTGDRVVBUS_NAME,			GPIO0A,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0A4_PWM1_NAME,				GPIO0A,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0A3_PWM0_NAME,				GPIO0A,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0A2_HDMII2CSDA_NAME,			GPIO0A,   4,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0A1_HDMII2CSCL_NAME,			GPIO0A,   2,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0A0_HDMIHOTPLUGIN_NAME, 			GPIO0A,   0,	 1,   0,	DEFAULT) 

//GPIO0B
MUX_CFG(GPIO0B7_I2S8CHSDO3_NAME,			GPIO0B,   14,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0B6_I2S8CHSDO2_NAME,			GPIO0B,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0B5_I2S8CHSDO1_NAME,			GPIO0B,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0B4_I2S8CHSDO0_NAME,			GPIO0B,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0B3_I2S8CHLRCKTX_NAME,			GPIO0B,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0B2_I2S8CHLRCKRX_NAME,			GPIO0B,   4,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO0B1_I2S8CHSCLK_NAME,			GPIO0B,   2,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0B0_I2S8CHCLK_NAME, 			GPIO0B,   0,	 1,   0,	DEFAULT)

//GPIO0C
MUX_CFG(GPIO0C7_TRACECTL_SMCADDR3_NAME, 		GPIO0C,   14,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0C6_TRACECLK_SMCADDR2_NAME, 		GPIO0C,   12,	 2,   0,	DEFAULT)
MUX_CFG(GPIO0C5_I2S12CHSDO_NAME,			GPIO0C,   10,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0C4_I2S12CHSDI_NAME,			GPIO0C,   8,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0C3_I2S12CHLRCKTX_NAME, 			GPIO0C,   6,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0C2_I2S12CHLRCKRX_NAME,			GPIO0C,   4,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0C1_I2S12CHSCLK_NAME,			GPIO0C,   2,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0C0_I2S12CHCLK_NAME,			GPIO0C,   0,	 1,   0,	DEFAULT)

//GPIO0D
MUX_CFG(GPIO0D7_PWM3_NAME,				GPIO0D,   14,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0D6_PWM2_NAME,				GPIO0D,   12,	 1,   0,	DEFAULT)
MUX_CFG(GPIO0D5_I2S22CHSDO_SMCADDR1_NAME,		GPIO0D,   10,	 2,   0,	DEFAULT)
MUX_CFG(GPIO0D4_I2S22CHSDI_SMCADDR0_NAME,		GPIO0D,   8,	 2,   0,	DEFAULT)
MUX_CFG(GPIO0D3_I2S22CHLRCKTX_SMCADVN_NAME, 		GPIO0D,   6,	 2,   0,	DEFAULT)
MUX_CFG(GPIO0D2_I2S22CHLRCKRX_SMCOEN_NAME,		GPIO0D,   4,	 2,   0,	DEFAULT)
MUX_CFG(GPIO0D1_I2S22CHSCLK_SMCWEN_NAME,		GPIO0D,   2,	 2,   0,	DEFAULT)
MUX_CFG(GPIO0D0_I2S22CHCLK_SMCCSN0_NAME,		GPIO0D,   0,	 2,   0,	DEFAULT)

//GPIO1A
MUX_CFG(GPIO1A7_UART1RTSN_SPI0TXD_NAME, 		GPIO1A,   14,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1A6_UART1CTSN_SPI0RXD_NAME, 		GPIO1A,   12,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1A5_UART1SOUT_SPI0CLK_NAME, 		GPIO1A,   10,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1A4_UART1SIN_SPI0CSN0_NAME, 		GPIO1A,   8,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1A3_UART0RTSN_NAME, 			GPIO1A,   6,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1A2_UART0CTSN_NAME, 			GPIO1A,   4,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1A1_UART0SOUT_NAME, 			GPIO1A,   2,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1A0_UART0SIN_NAME,				GPIO1A,   0,	 1,   0,	DEFAULT)

//GPIO1B
MUX_CFG(GPIO1B7_CIFDATA11_NAME, 			GPIO1B,   14,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1B6_CIFDATA10_NAME, 			GPIO1B,   12,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1B5_CIF0DATA1_NAME, 			GPIO1B,   10,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1B4_CIF0DATA0_NAME, 			GPIO1B,   8,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1B3_CIF0CLKOUT_NAME,			GPIO1B,   6,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1B2_SPDIFTX_NAME,				GPIO1B,   4,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1B1_UART2SOUT_NAME, 			GPIO1B,   2,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1B0_UART2SIN_NAME,				GPIO1B,   0,	 1,   0,	DEFAULT)

//GPIO1C
MUX_CFG(GPIO1C7_CIFDATA9_RMIIRXD0_NAME, 		GPIO1C,   14,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1C6_CIFDATA8_RMIIRXD1_NAME, 		GPIO1C,   12,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1C5_CIFDATA7_RMIICRSDVALID_NAME,		GPIO1C,   10,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1C4_CIFDATA6_RMIIRXERR_NAME,		GPIO1C,   8,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1C3_CIFDATA5_RMIITXD0_NAME, 		GPIO1C,   6,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1C2_CIF1DATA4_RMIITXD1_NAME,		GPIO1C,   4,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO1C1_CIFDATA3_RMIITXEN_NAME, 		GPIO1C,   2,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1C0_CIF1DATA2_RMIICLKOUT_RMIICLKIN_NAME,	GPIO1C,   0,	 2,   0,	DEFAULT)

//GPIO1D
MUX_CFG(GPIO1D7_CIF1CLKOUT_NAME,			GPIO1D,   14,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1D6_CIF1DATA11_NAME, 			GPIO1D,   12,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1D5_CIF1DATA10_NAME,			GPIO1D,   10,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1D4_CIF1DATA1_NAME, 			GPIO1D,   8,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1D3_CIF1DATA0_NAME, 			GPIO1D,   6,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1D2_CIF1CLKIN_NAME, 			GPIO1D,   4,	 1,   0,	DEFAULT)
MUX_CFG(GPIO1D1_CIF1HREF_MIIMDCLK_NAME, 		GPIO1D,   2,	 2,   0,	DEFAULT)
MUX_CFG(GPIO1D0_CIF1VSYNC_MIIMD_NAME,			GPIO1D,   0,	 2,   0,	DEFAULT)

//GPIO2A
MUX_CFG(GPIO2A7_LCDC1DATA7_SMCADDR11_NAME,		GPIO2A,   14,	 2,   0,	DEFAULT)
MUX_CFG(GPIO2A6_LCDC1DATA6_SMCADDR10_NAME,		GPIO2A,   12,	 2,   0,	DEFAULT)
MUX_CFG(GPIO2A5_LCDC1DATA5_SMCADDR9_NAME,		GPIO2A,   10,	 2,   0,	DEFAULT)
MUX_CFG(GPIO2A4_LCDC1DATA4_SMCADDR8_NAME,		GPIO2A,   8,	 2,   0,	DEFAULT)
MUX_CFG(GPIO2A3_LCDCDATA3_SMCADDR7_NAME,		GPIO2A,   6,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2A2_LCDCDATA2_SMCADDR6_NAME,		GPIO2A,   4,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2A1_LCDC1DATA1_SMCADDR5_NAME,		GPIO2A,   2,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2A0_LCDC1DATA0_SMCADDR4_NAME,		GPIO2A,   0,	 2,   0,	DEFAULT) 

//GPIO2B
MUX_CFG(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME,	GPIO2B,   14,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME,	GPIO2B,   12,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2B5_LCDC1DATA13_SMCADDR17_HSADCDATA8_NAME,	GPIO2B,   10,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2B4_LCDC1DATA12_SMCADDR16_HSADCDATA9_NAME,	GPIO2B,   8,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2B3_LCDC1DATA11_SMCADDR15_NAME, 		GPIO2B,   6,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2B2_LCDC1DATA10_SMCADDR14_NAME, 		GPIO2B,   4,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2B1_LCDC1DATA9_SMCADDR13_NAME,		GPIO2B,   2,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2B0_LCDC1DATA8_SMCADDR12_NAME,		GPIO2B,   0,	 2,   0,	DEFAULT) 

//GPIO2C
MUX_CFG(GPIO2C7_LCDC1DATA23_SPI1CSN1_HSADCDATA4_NAME,	GPIO2C,   14,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2C6_LCDC1DATA22_SPI1RXD_HSADCDATA3_NAME,	GPIO2C,   12,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2C5_LCDC1DATA21_SPI1TXD_HSADCDATA2_NAME,	GPIO2C,   10,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2C4_LCDC1DATA20_SPI1CSN0_HSADCDATA1_NAME,	GPIO2C,   8,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME,	GPIO2C,   6,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2C2_LCDC1DATA18_SMCBLSN1_HSADCDATA5_NAME,	GPIO2C,   4,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2C1_LCDC1DATA17_SMCBLSN0_HSADCDATA6_NAME,	GPIO2C,   2,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, 	GPIO2C,   0,	 2,   0,	DEFAULT) 

//GPIO2D
MUX_CFG(GPIO2D7_I2C1SCL_NAME,				GPIO2D,   14,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO2D6_I2C1SDA_NAME,				GPIO2D,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO2D5_I2C0SCL_NAME,				GPIO2D,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO2D4_I2C0SDA_NAME,				GPIO2D,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO2D3_LCDC1VSYNC_NAME,			GPIO2D,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO2D2_LCDC1HSYNC_NAME,			GPIO2D,   4,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO2D1_LCDC1DEN_SMCCSN1_NAME,			GPIO2D,   2,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO2D0_LCDC1DCLK_NAME, 			GPIO2D,   0,	 1,   0,	DEFAULT) 

//GPIO3A
MUX_CFG(GPIO3A7_SDMMC0PWREN_NAME,			GPIO3A,   14,	 1,   0,	DEFAULT) //Modifyed by xbw,at 2012-03-05
MUX_CFG(GPIO3A6_SDMMC0RSTNOUT_NAME, 			GPIO3A,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3A5_I2C4SCL_NAME,				GPIO3A,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3A4_I2C4SDA_NAME,				GPIO3A,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3A3_I2C3SCL_NAME,				GPIO3A,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3A2_I2C3SDA_NAME,				GPIO3A,   4,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3A1_I2C2SCL_NAME,				GPIO3A,   2,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3A0_I2C2SDA_NAME,				GPIO3A,   0,	 1,   0,	DEFAULT) 

//GPIO3B
MUX_CFG(GPIO3B7_SDMMC0WRITEPRT_NAME,			GPIO3B,   14,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3B6_SDMMC0DETECTN_NAME, 			GPIO3B,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3B5_SDMMC0DATA3_NAME,			GPIO3B,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3B4_SDMMC0DATA2_NAME,			GPIO3B,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3B3_SDMMC0DATA1_NAME,			GPIO3B,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3B2_SDMMC0DATA0_NAME,			GPIO3B,   4,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3B1_SDMMC0CMD_NAME, 			GPIO3B,   2,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3B0_SDMMC0CLKOUT_NAME,			GPIO3B,   0,	 1,   0,	DEFAULT) 

//GPIO3C
MUX_CFG(GPIO3C7_SDMMC1WRITEPRT_NAME,			GPIO3C,   14,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3C6_SDMMC1DETECTN_NAME, 			GPIO3C,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3C5_SDMMC1CLKOUT_NAME,			GPIO3C,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3C4_SDMMC1DATA3_NAME,			GPIO3C,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3C3_SDMMC1DATA2_NAME,			GPIO3C,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3C2_SDMMC1DATA1_NAME,			GPIO3C,   4,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3C1_SDMMC1DATA0_NAME,			GPIO3C,   2,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3C0_SMMC1CMD_NAME,				GPIO3C,   0,	 1,   0,	DEFAULT) 

//GPIO3D
MUX_CFG(GPIO3D7_FLASHDQS_EMMCCLKOUT_NAME,		GPIO3D,   14,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO3D6_UART3RTSN_NAME, 			GPIO3D,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3D5_UART3CTSN_NAME, 			GPIO3D,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3D4_UART3SOUT_NAME, 			GPIO3D,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3D3_UART3SIN_NAME,				GPIO3D,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3D2_SDMMC1INTN_NAME,			GPIO3D,   4,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3D1_SDMMC1BACKENDPWR_NAME,			GPIO3D,   2,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO3D0_SDMMC1PWREN_NAME,			GPIO3D,   0,	 1,   0,	DEFAULT) 

//GPIO4A
MUX_CFG(GPIO4A7_FLASHDATA15_NAME,			GPIO4A,   14,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4A6_FLASHDATA14_NAME,			GPIO4A,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4A5_FLASHDATA13_NAME,			GPIO4A,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4A4_FLASHDATA12_NAME,			GPIO4A,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4A3_FLASHDATA11_NAME,			GPIO4A,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4A2_FLASHDATA10_NAME,			GPIO4A,   4,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4A1_FLASHDATA9_NAME,			GPIO4A,   2,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4A0_FLASHDATA8_NAME,			GPIO4A,   0,	 1,   0,	DEFAULT) 

//GPIO4B
MUX_CFG(GPIO4B7_SPI0CSN1_NAME,				GPIO4B,   14,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4B6_FLASHCSN7_NAME, 			GPIO4B,   12,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4B5_FLASHCSN6_NAME, 			GPIO4B,   10,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4B4_FLASHCSN5_NAME, 			GPIO4B,   8,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4B3_FLASHCSN4_NAME, 			GPIO4B,   6,	 1,   0,	DEFAULT) 
MUX_CFG(GPIO4B2_FLASHCSN3_EMMCRSTNOUT_NAME, 		GPIO4B,   4,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4B1_FLASHCSN2_EMMCCMD_NAME, 		GPIO4B,   2,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4B0_FLASHCSN1_NAME, 			GPIO4B,   0,	 1,   0,	DEFAULT) 

//GPIO4C
MUX_CFG(GPIO4C7_SMCDATA7_TRACEDATA7_NAME,		GPIO4C,   14,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4C6_SMCDATA6_TRACEDATA6_NAME,		GPIO4C,   12,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4C5_SMCDATA5_TRACEDATA5_NAME,		GPIO4C,   10,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4C4_SMCDATA4_TRACEDATA4_NAME,		GPIO4C,   8,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4C3_SMCDATA3_TRACEDATA3_NAME,		GPIO4C,   6,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4C2_SMCDATA2_TRACEDATA2_NAME,		GPIO4C,   4,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4C1_SMCDATA1_TRACEDATA1_NAME,		GPIO4C,   2,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4C0_SMCDATA0_TRACEDATA0_NAME,		GPIO4C,   0,	 2,   0,	DEFAULT) 

//GPIO4D
MUX_CFG(GPIO4D7_SMCDATA15_TRACEDATA15_NAME, 		GPIO4D,   14,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4D6_SMCDATA14_TRACEDATA14_NAME, 		GPIO4D,   12,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4D5_SMCDATA13_TRACEDATA13_NAME, 		GPIO4D,   10,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4D4_SMCDATA12_TRACEDATA12_NAME, 		GPIO4D,   8,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4D3_SMCDATA11_TRACEDATA11_NAME, 		GPIO4D,   6,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, 		GPIO4D,   4,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4D1_SMCDATA9_TRACEDATA9_NAME,		GPIO4D,   2,	 2,   0,	DEFAULT) 
MUX_CFG(GPIO4D0_SMCDATA8_TRACEDATA8_NAME,		GPIO4D,   0,	 2,   0,	DEFAULT) 

//GPIO6B
MUX_CFG(GPIO6B7_TESTCLOCKOUT_NAME,			GPIO6B,   14,	 1,   0,	DEFAULT) 

};     


void rk30_mux_set(struct mux_config *cfg)
{
	int regValue = 0;
	int mask;
	
	mask = (((1<<(cfg->interleave))-1)<<cfg->offset) << 16;
	regValue |= mask;
	regValue |=(cfg->mode<<cfg->offset);
#ifdef IOMUX_DBG
	printk("%s::reg=0x%p,Value=0x%x,mask=0x%x\n",__FUNCTION__,cfg->mux_reg,regValue,mask);
#endif
	writel_relaxed(regValue,cfg->mux_reg);
	dsb();
	
	return;
}


int __init rk30_iomux_init(void)
{
	int i;
	printk("%s\n",__func__);
	for(i=0;i<ARRAY_SIZE(rk30_muxs);i++)
	{
		if(rk30_muxs[i].flags != DEFAULT)
			rk30_mux_set(&rk30_muxs[i]);	
	}

#if defined(CONFIG_UART0_RK29) || (CONFIG_RK_DEBUG_UART == 0)
	rk30_mux_api_set(GPIO1A1_UART0SOUT_NAME, GPIO1A_UART0_SOUT);
	rk30_mux_api_set(GPIO1A0_UART0SIN_NAME, GPIO1A_UART0_SIN);
#ifdef CONFIG_UART0_CTS_RTS_RK29
	rk30_mux_api_set(GPIO1A3_UART0RTSN_NAME, GPIO1A_UART0_RTS_N);
	rk30_mux_api_set(GPIO1A2_UART0CTSN_NAME, GPIO1A_UART0_CTS_N);
#endif
#endif
#if defined(CONFIG_UART1_RK29) || (CONFIG_RK_DEBUG_UART == 1)
	//UART1 OR SPIM0
	rk30_mux_api_set(GPIO1A5_UART1SOUT_SPI0CLK_NAME, GPIO1A_UART1_SOUT);
	rk30_mux_api_set(GPIO1A4_UART1SIN_SPI0CSN0_NAME, GPIO1A_UART1_SIN);
#ifdef CONFIG_UART1_CTS_RTS_RK29
	rk30_mux_api_set(GPIO1A7_UART1RTSN_SPI0TXD_NAME, GPIO1A_UART1_RTS_N);
	rk30_mux_api_set(GPIO1A6_UART1CTSN_SPI0RXD_NAME, GPIO1A_UART1_CTS_N);
#endif
#endif

#if defined(CONFIG_UART2_RK29) || (CONFIG_RK_DEBUG_UART == 2)
	rk30_mux_api_set(GPIO1B1_UART2SOUT_NAME, GPIO1B_UART2_SOUT);
	rk30_mux_api_set(GPIO1B0_UART2SIN_NAME, GPIO1B_UART2_SIN);
#endif
#if defined(CONFIG_UART3_RK29) || (CONFIG_RK_DEBUG_UART == 3)
	rk30_mux_api_set(GPIO3D4_UART3SOUT_NAME, GPIO3D_UART3_SOUT);
	rk30_mux_api_set(GPIO3D3_UART3SIN_NAME, GPIO3D_UART3_SIN);
#ifdef CONFIG_UART3_CTS_RTS_RK29
	rk30_mux_api_set(GPIO3D6_UART3RTSN_NAME, GPIO3D_UART3_RTS_N);
	rk30_mux_api_set(GPIO3D5_UART3CTSN_NAME, GPIO3D_UART3_CTS_N);
#endif
#endif
#ifdef CONFIG_SPIM0_RK29
	//UART1 OR SPIM0
	rk30_mux_api_set(GPIO1A5_UART1SOUT_SPI0CLK_NAME, GPIO1A_SPI0_CLK);
	rk30_mux_api_set(GPIO1A4_UART1SIN_SPI0CSN0_NAME, GPIO1A_SPI0_CSN0);
	rk30_mux_api_set(GPIO1A7_UART1RTSN_SPI0TXD_NAME, GPIO1A_SPI0_TXD);
	rk30_mux_api_set(GPIO1A6_UART1CTSN_SPI0RXD_NAME, GPIO1A_SPI0_RXD);
#endif
#ifdef CONFIG_SPIM1_RK29
	//rk30_mux_api_set(GPIO2C7_LCDC1DATA23_SPI1CSN1_HSADCDATA4_NAME, GPIO2C_SPI1_CSN1);
	rk30_mux_api_set(GPIO2C6_LCDC1DATA22_SPI1RXD_HSADCDATA3_NAME, GPIO2C_SPI1_RXD);
	rk30_mux_api_set(GPIO2C5_LCDC1DATA21_SPI1TXD_HSADCDATA2_NAME, GPIO2C_SPI1_TXD);
	rk30_mux_api_set(GPIO2C4_LCDC1DATA20_SPI1CSN0_HSADCDATA1_NAME, GPIO2C_SPI1_CSN0);
	rk30_mux_api_set(GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME, GPIO2C_SPI1_CLK);
#endif

#ifdef CONFIG_I2C0_RK30
	rk30_mux_api_set(GPIO2D5_I2C0SCL_NAME, GPIO2D_I2C0_SCL);	
	rk30_mux_api_set(GPIO2D4_I2C0SDA_NAME, GPIO2D_I2C0_SDA);
#endif

#ifdef CONFIG_I2C1_RK30
	rk30_mux_api_set(GPIO2D7_I2C1SCL_NAME, GPIO2D_I2C1_SCL);
	rk30_mux_api_set(GPIO2D6_I2C1SDA_NAME, GPIO2D_I2C1_SDA);
#endif

#ifdef CONFIG_I2C2_RK30
	rk30_mux_api_set(GPIO3A1_I2C2SCL_NAME, GPIO3A_I2C2_SCL);
	rk30_mux_api_set(GPIO3A0_I2C2SDA_NAME, GPIO3A_I2C2_SDA);
#endif

#ifdef CONFIG_I2C3_RK30
	rk30_mux_api_set(GPIO3A3_I2C3SCL_NAME, GPIO3A_I2C3_SCL);
	rk30_mux_api_set(GPIO3A2_I2C3SDA_NAME, GPIO3A_I2C3_SDA);
#endif

#ifdef CONFIG_I2C4_RK30
	rk30_mux_api_set(GPIO3A5_I2C4SCL_NAME, GPIO3A_I2C4_SCL);
	rk30_mux_api_set(GPIO3A4_I2C4SDA_NAME, GPIO3A_I2C4_SDA);
#endif

#ifdef CONFIG_RK30_VMAC
	rk30_mux_api_set(GPIO1C0_CIF1DATA2_RMIICLKOUT_RMIICLKIN_NAME, GPIO1C_RMII_CLKOUT);
	rk30_mux_api_set(GPIO1C1_CIFDATA3_RMIITXEN_NAME, GPIO1C_RMII_TX_EN);
	rk30_mux_api_set(GPIO1C2_CIF1DATA4_RMIITXD1_NAME, GPIO1C_RMII_TXD1);
//	rk30_mux_api_set(GPIO1C3_CIFDATA5_RMIITXD0_NAME, GPIO1C_RMII_TXD0);ltm++
	rk30_mux_api_set(GPIO1C4_CIFDATA6_RMIIRXERR_NAME, GPIO1C_RMII_RX_ERR);
	rk30_mux_api_set(GPIO1C5_CIFDATA7_RMIICRSDVALID_NAME, GPIO1C_RMII_CRS_DVALID);
	rk30_mux_api_set(GPIO1C6_CIFDATA8_RMIIRXD1_NAME, GPIO1C_RMII_RXD1);
	rk30_mux_api_set(GPIO1C7_CIFDATA9_RMIIRXD0_NAME, GPIO1C_RMII_RXD0);

	rk30_mux_api_set(GPIO1D1_CIF1HREF_MIIMDCLK_NAME, GPIO1D_MII_MDCLK);
	rk30_mux_api_set(GPIO1D0_CIF1VSYNC_MIIMD_NAME, GPIO1D_MII_MD);
#endif

	return 0;
}

/*
 *config iomux : input iomux name and iomux flags
 */ 
void rk30_mux_api_set(char *name, unsigned int mode)
{
  int i;
        if (!name) {
                return;
        } 
	for(i=0;i<ARRAY_SIZE(rk30_muxs);i++)
	{
		if (!strcmp(rk30_muxs[i].name, name))
		{
		    rk30_muxs[i].premode = rk30_muxs[i].mode;
			rk30_muxs[i].mode = mode;
			rk30_mux_set(&rk30_muxs[i]);	
			break;			
		}
	}
}
EXPORT_SYMBOL(rk30_mux_api_set);


int rk30_mux_api_get(char *name)
{
	int i,ret=0;
	if (!name) {
		return -1;
	}
	for(i=0;i<ARRAY_SIZE(rk30_muxs);i++)
	{
		if (!strcmp(rk30_muxs[i].name, name))
		{
			ret = readl(rk30_muxs[i].mux_reg);
			ret = (ret >> rk30_muxs[i].offset) &((1<<(rk30_muxs[i].interleave))-1);
			return ret;
		}
	}

	return -1;
}
EXPORT_SYMBOL(rk30_mux_api_get);
