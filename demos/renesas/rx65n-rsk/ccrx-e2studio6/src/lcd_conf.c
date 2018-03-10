/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2015-2017 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : main.c
* Description  : sample program of TSIP main program.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 26.08.2017 1.00    First Release
***********************************************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "r_simple_graphic_if.h"

#if SECURE_BOOT
#pragma section SECURE_BOOT
#endif

#define LCD_X_INITCONTROLLER 0x01 /* Initializing the display controller */
#define LCD_X_SETLUTENTRY    0x04 /* Setting an entry of the LUT */
#define LCD_X_ON             0x05 /* Switching the display on */

#define XSIZE_PHYS 480
#define YSIZE_PHYS 272

//
// Timing
//
#define SYNC_H       4
#define SYNC_V       1
#define BGSYNC_HP    2
#define BGSYNC_VP    2
#define BGHSIZE_HP  (BGSYNC_HP + SYNC_H)
#define BGVSIZE_VP  (BGSYNC_VP + SYNC_V)
#define GRC_VS      (BGVSIZE_VP - BGSYNC_VP)
#define GRC_HS      (BGHSIZE_HP - BGSYNC_HP)

// Color depth
//
#define BITS_PER_PIXEL 8  // Allowed values: 1, 4, 8, 16, 32

//
// Frame buffer
//
#define FRAMEBUFFER_START 0x800000  // Begin of Expansion RAM

//
// Buffer size and stride
//
#define BYTES_PER_LINE   ((BITS_PER_PIXEL * XSIZE_PHYS) / 8)
#define LINE_OFFSET      (((BYTES_PER_LINE + 63) / 64) * 64)
#define VXSIZE_PHYS      ((LINE_OFFSET * 8) / BITS_PER_PIXEL)
#define BYTES_PER_BUFFER (LINE_OFFSET * YSIZE_PHYS)

//
// Color format selection
//
#define FORMAT_RGB_565   0
#define FORMAT_RGB_888   1
#define FORMAT_ARGB_1555 2
#define FORMAT_ARGB_4444 3
#define FORMAT_ARGB_8888 4
#define FORMAT_CLUT_8    5
#define FORMAT_CLUT_4    6
#define FORMAT_CLUT_1    7

#define COLOR_CONVERSION GUICC_8666
#define DISPLAY_DRIVER   GUIDRV_LIN_8
#define COLOR_FORMAT     FORMAT_CLUT_8
#define NUM_BUFFERS      2

//
// Brightness & COntrast
//
#define _BRIGHTNESS 0x200
#define _CONTRAST   0x80

typedef uint32_t LCD_COLOR;

typedef struct {
  LCD_COLOR Color;
  uint8_t Pos;
} LCD_X_SETLUTENTRY_INFO;

const uint32_t lcduteconf[256] =
{
		0xFF000000, 0xFF330000, 0xFF660000, 0xFF990000,
		0xFFCC0000, 0xFFFF0000, 0xFF003300, 0xFF333300,
		0xFF663300, 0xFF993300, 0xFFCC3300, 0xFFFF3300,
		0xFF006600, 0xFF336600, 0xFF666600, 0xFF996600,
		0xFFCC6600, 0xFFFF6600, 0xFF009900, 0xFF339900,
		0xFF669900, 0xFF999900, 0xFFCC9900, 0xFFFF9900,
		0xFF00CC00, 0xFF33CC00, 0xFF66CC00, 0xFF99CC00,
		0xFFCCCC00, 0xFFFFCC00, 0xFF00FF00, 0xFF33FF00,
		0xFF66FF00, 0xFF99FF00, 0xFFCCFF00, 0xFFFFFF00,
		0xFF000033, 0xFF330033, 0xFF660033, 0xFF990033,
		0xFFCC0033, 0xFFFF0033, 0xFF003333, 0xFF333333,
		0xFF663333, 0xFF993333, 0xFFCC3333, 0xFFFF3333,
		0xFF006633, 0xFF336633, 0xFF666633, 0xFF996633,
		0xFFCC6633, 0xFFFF6633, 0xFF009933, 0xFF339933,
		0xFF669933, 0xFF999933, 0xFFCC9933, 0xFFFF9933,
		0xFF00CC33, 0xFF33CC33, 0xFF66CC33, 0xFF99CC33,
		0xFFCCCC33, 0xFFFFCC33, 0xFF00FF33, 0xFF33FF33,
		0xFF66FF33, 0xFF99FF33, 0xFFCCFF33, 0xFFFFFF33,
		0xFF000066, 0xFF330066, 0xFF660066, 0xFF990066,
		0xFFCC0066, 0xFFFF0066, 0xFF003366, 0xFF333366,
		0xFF663366, 0xFF993366, 0xFFCC3366, 0xFFFF3366,
		0xFF006666, 0xFF336666, 0xFF666666, 0xFF996666,
		0xFFCC6666, 0xFFFF6666, 0xFF009966, 0xFF339966,
		0xFF669966, 0xFF999966, 0xFFCC9966, 0xFFFF9966,
		0xFF00CC66, 0xFF33CC66, 0xFF66CC66, 0xFF99CC66,
		0xFFCCCC66, 0xFFFFCC66, 0xFF00FF66, 0xFF33FF66,
		0xFF66FF66, 0xFF99FF66, 0xFFCCFF66, 0xFFFFFF66,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0xFF000000, 0xFF111111, 0xFF222222, 0xFF333333,
		0xFF444444, 0xFF555555, 0xFF666666, 0xFF777777,
		0xFF888888, 0xFF999999, 0xFFAAAAAA, 0xFFBBBBBB,
		0xFFCCCCCC, 0xFFDDDDDD, 0xFFEEEEEE, 0xFFFFFFFF,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0xFF000099, 0xFF330099, 0xFF660099, 0xFF990099,
		0xFFCC0099, 0xFFFF0099, 0xFF003399, 0xFF333399,
		0xFF663399, 0xFF993399, 0xFFCC3399, 0xFFFF3399,
		0xFF006699, 0xFF336699, 0xFF666699, 0xFF996699,
		0xFFCC6699, 0xFFFF6699, 0xFF009999, 0xFF339999,
		0xFF669999, 0xFF999999, 0xFFCC9999, 0xFFFF9999,
		0xFF00CC99, 0xFF33CC99, 0xFF66CC99, 0xFF99CC99,
		0xFFCCCC99, 0xFFFFCC99, 0xFF00FF99, 0xFF33FF99,
		0xFF66FF99, 0xFF99FF99, 0xFFCCFF99, 0xFFFFFF99,
		0xFF0000CC, 0xFF3300CC, 0xFF6600CC, 0xFF9900CC,
		0xFFCC00CC, 0xFFFF00CC, 0xFF0033CC, 0xFF3333CC,
		0xFF6633CC, 0xFF9933CC, 0xFFCC33CC, 0xFFFF33CC,
		0xFF0066CC, 0xFF3366CC, 0xFF6666CC, 0xFF9966CC,
		0xFFCC66CC, 0xFFFF66CC, 0xFF0099CC, 0xFF3399CC,
		0xFF6699CC, 0xFF9999CC, 0xFFCC99CC, 0xFFFF99CC,
		0xFF00CCCC, 0xFF33CCCC, 0xFF66CCCC, 0xFF99CCCC,
		0xFFCCCCCC, 0xFFFFCCCC, 0xFF00FFCC, 0xFF33FFCC,
		0xFF66FFCC, 0xFF99FFCC, 0xFFCCFFCC, 0xFFFFFFCC,
		0xFF0000FF, 0xFF3300FF, 0xFF6600FF, 0xFF9900FF,
		0xFFCC00FF, 0xFFFF00FF, 0xFF0033FF, 0xFF3333FF,
		0xFF6633FF, 0xFF9933FF, 0xFFCC33FF, 0xFFFF33FF,
		0xFF0066FF, 0xFF3366FF, 0xFF6666FF, 0xFF9966FF,
		0xFFCC66FF, 0xFFFF66FF, 0xFF0099FF, 0xFF3399FF,
		0xFF6699FF, 0xFF9999FF, 0xFFCC99FF, 0xFFFF99FF,
		0xFF00CCFF, 0xFF33CCFF, 0xFF66CCFF, 0xFF99CCFF,
		0xFFCCCCFF, 0xFFFFCCFF, 0xFF00FFFF, 0xFF33FFFF,
		0xFF66FFFF, 0xFF99FFFF, 0xFFCCFFFF, 0xFFFFFFFF,
};

static volatile int _PendingBuffer = -1;
static const uint32_t _aBufferPTR[] = {
  0x00000000,  // Begin of On-Chip RAM
  0x00800000   // Begin of Expansion RAM
};

void lcd_open(void);
void lcd_close(void);
uint32_t* lcd_get_frame_buffer_pointer(void);

static void _VSYNC_ISR(void * p);
static void _SetLUTEntry(LCD_COLOR Color, uint8_t Pos);
static void _DisplayOnOff(int OnOff);
static void LCD_shutdownController(void);

#pragma section _FRAME1
//uint8_t frame_buffer1[256*1024];	/* this sample does not use this one */
#pragma section _FRAME2
uint8_t frame_buffer2[256*1024];
#pragma section

#if SECURE_BOOT
#pragma section SECURE_BOOT
#endif

void lcd_open(void)
{
    LCD_X_SETLUTENTRY_INFO colorcnf;
    uint32_t i;

	//
	// Release stop state of GLCDC
	//
	SYSTEM.PRCR.WORD = 0xA50Bu;      // Protect off
	SYSTEM.MSTPCRC.BIT.MSTPC29 = 0;  // GLCDC, Release from the module-stop state
	SYSTEM.PRCR.WORD = 0xA500u;      // Protect on
	//
	// Function select of multiplex pins (Display B)
	//
	  MPC.PWPR.BIT.B0WI   = 0;     // Enable function select access
	  MPC.PWPR.BIT.PFSWE  = 1;
	  MPC.PB0PFS.BIT.PSEL = 0x25;  // Pin8  - R3
	  MPC.PA7PFS.BIT.PSEL = 0x25;  // Pin9  - R4
	  MPC.PA6PFS.BIT.PSEL = 0x25;  // Pin10 - R5 (R0)
	  MPC.PA5PFS.BIT.PSEL = 0x25;  // Pin11 - R6 (R1)
	  MPC.PA4PFS.BIT.PSEL = 0x25;  // Pin12 - R7 (R2)
	  MPC.PA3PFS.BIT.PSEL = 0x25;  // Pin15 - G2
	  MPC.PA2PFS.BIT.PSEL = 0x25;  // Pin16 - G3
	  MPC.PA1PFS.BIT.PSEL = 0x25;  // Pin17 - G4
	  MPC.PA0PFS.BIT.PSEL = 0x25;  // Pin18 - G5
	  MPC.PE7PFS.BIT.PSEL = 0x25;  // Pin19 - G6 (G0)
	  MPC.PE6PFS.BIT.PSEL = 0x25;  // Pin20 - G7 (G1)
	  MPC.PE5PFS.BIT.PSEL = 0x25;  // Pin24 - B3
	  MPC.PE4PFS.BIT.PSEL = 0x25;  // Pin25 - B4
	  MPC.PE3PFS.BIT.PSEL = 0x25;  // Pin26 - B5 (B0)
	  MPC.PE2PFS.BIT.PSEL = 0x25;  // Pin27 - B6 (B1)
	  MPC.PE1PFS.BIT.PSEL = 0x25;  // Pin28 - B7 (B2)
	  MPC.PB5PFS.BIT.PSEL = 0x25;  // Pin30 - CLK
	  MPC.PB2PFS.BIT.PSEL = 0x25;  // Pin32 - HSYNC (TCON2)
	  MPC.PB4PFS.BIT.PSEL = 0x25;  // Pin33 - VSYNC (TCON0)
	  MPC.PB1PFS.BIT.PSEL = 0x25;  // Pin34 - DEN   (TCON3)
	  MPC.PWPR.BIT.B0WI   = 0;     // Disable function select access
	  MPC.PWPR.BIT.PFSWE  = 0;
	//
	// Select IO function (Port Mode Register)
	//
	SYSTEM.PRCR.WORD = 0xA50Bu;  // Protect off
	PORTB.PMR.BYTE   = 0x37;     // Pins 0, 1, 2, 4 and 5 of PORTB
	PORTA.PMR.BYTE   = 0xFF;     // Pins 0 - 7 of PORTA
	PORTE.PMR.BYTE   = 0xFE;     // Pins 1 - 7 of PORTE
	SYSTEM.PRCR.WORD = 0xA500u;  // Protect on
	//
	// Set DISP signal on P97 (Pin31)
	//
	  PORT9.PDR.BIT.B7  = 1;  // Port direction: output
	  PORT9.PODR.BIT.B7 = 1;  // DISP on
	//
	// Switch backlight on, no PWM
	//
	  PORTB.PDR.BIT.B7  = 1;
	  PORTB.PODR.BIT.B7 = 1;
	//
	// Set the BGEN.SWRST bit to 1 to release the GLCDC from a software reset
	//
	GLCDC.BGEN.BIT.SWRST = 1;
	//
	// Set the frequency of the LCD_CLK and PXCLK to suit the format and set the PANELCLK.CLKEN bit to 1
	//
	GLCDC.PANELCLK.BIT.CLKSEL = 1;   // Select PLL clock
	GLCDC.PANELCLK.BIT.DCDR   = 24;  // 240 / 24 = 10 MHz
	GLCDC.PANELCLK.BIT.PIXSEL = 0;   // No frequency division
	GLCDC.PANELCLK.BIT.CLKEN  = 1;   // Enable LCD_CLK output
	//
	// Definition of Background Screen
	//
	GLCDC.BGPERI.BIT.FH  = XSIZE_PHYS + 45;  // Horizontal cycle (whole control screen)
	GLCDC.BGPERI.BIT.FV  = YSIZE_PHYS + 14;  // Vertical cycle (whole control screen)
	GLCDC.BGSYNC.BIT.HP  = BGSYNC_HP;        // Horizontal Synchronization Signal Assertion Position
	GLCDC.BGSYNC.BIT.VP  = BGSYNC_VP;        // Vertical Synchronization Assertion Position
	GLCDC.BGHSIZE.BIT.HP = BGHSIZE_HP;       // Horizontal Active Pixel Start Position (min. 6 pixels)
	GLCDC.BGHSIZE.BIT.HW = XSIZE_PHYS;       // Horizontal Active Pixel Width
	GLCDC.BGVSIZE.BIT.VP = BGVSIZE_VP;       // Vertical Active Display Start Position (min. 3 lines)
	GLCDC.BGVSIZE.BIT.VW = YSIZE_PHYS;       // Vertical Active Display Width
	//
	// Graphic 2 configuration
	//
	GLCDC.GR2FLMRD.BIT.RENB    = 1;                     // Enable reading of the frame buffer
	GLCDC.GR2FLM2              = FRAMEBUFFER_START;     // Specify the start address of the frame buffer
	GLCDC.GR2FLM3.BIT.LNOFF    = LINE_OFFSET;           // Offset value from the end address of the line to the start address of the next line
	GLCDC.GR2FLM5.BIT.DATANUM  = LINE_OFFSET / 64 - 1;  // Single Line Data Transfer Count
	GLCDC.GR2FLM5.BIT.LNNUM    = YSIZE_PHYS - 1;        // Single Frame Line Count
	GLCDC.GR2FLM6.BIT.FORMAT   = COLOR_FORMAT;          // Frame Buffer Color Format RGB565
	GLCDC.GR2AB1.BIT.DISPSEL   = 2;                     // Display Screen Control (current graphics)
	GLCDC.GR2AB1.BIT.ARCDISPON = 0;                     // Rectangular Alpha Blending Area Frame Display Control
	GLCDC.GR2AB1.BIT.GRCDISPON = 0;                     // Graphics Area Frame Display Control
	GLCDC.GR2AB1.BIT.ARCON     = 0;                     // Alpha Blending Control (Per-pixel alpha blending)
	GLCDC.GR2AB2.BIT.GRCVS     = GRC_VS;                // Graphics Area Vertical Start Position
	GLCDC.GR2AB2.BIT.GRCVW     = YSIZE_PHYS;            // Graphics Area Vertical Width
	GLCDC.GR2AB3.BIT.GRCHS     = GRC_HS;                // Graphics Area Horizontal Start Position
	GLCDC.GR2AB3.BIT.GRCHW     = XSIZE_PHYS;            // Graphics Area Horizontal Width
	GLCDC.GR2AB4.BIT.ARCVS     = GRC_VS;                // Rectangular Alpha Blending Area Vertical Start Position
	GLCDC.GR2AB4.BIT.ARCVW     = YSIZE_PHYS;            // Rectangular Alpha Blending Area Vertical Width
	GLCDC.GR2AB5.BIT.ARCHS     = GRC_HS;                // Rectangular Alpha Blending Area Horizontal Start Position
	GLCDC.GR2AB5.BIT.ARCHW     = XSIZE_PHYS;            // Rectangular Alpha Blending Area Horizontal Width
	GLCDC.GR2VEN.BIT.VEN       = 1;                     // Graphic 2 Register Value Reflection Enable
	//
	// Timing configuration
	//
	GLCDC.TCONTIM.BIT.OFFSET  = 3;           // Horizontal Synchronization Signal Reference Timing Offset
	GLCDC.TCONSTHB2.BIT.HSSEL = 1;           // Signal Reference Timing Select
	GLCDC.TCONSTVB1.BIT.VS    = 1;           // STVB Signal Assertion Timing
	GLCDC.TCONSTVB1.BIT.VW    = YSIZE_PHYS;  // STVB Signal Pulse Width
	GLCDC.TCONSTHB1.BIT.HW    = XSIZE_PHYS;  // STHB Signal Pulse Width
	GLCDC.TCONSTVA2.BIT.SEL   = 0;           // TCON0 Output Signal Select STVA (VSYNC)
	GLCDC.TCONSTHA2.BIT.SEL   = 2;           // TCON2 Output Signal Select STHA (HSYNC)
	GLCDC.TCONSTHB2.BIT.SEL   = 7;           // TCON3 Output Signal Select DE (DEN)
	//
	// Output interface
	//
	GLCDC.OUTSET.BIT.PHASE    = 0;  // Serial RGB Data Output Delay Control (0 cycles)
	GLCDC.OUTSET.BIT.DIRSEL   = 0;  // Serial RGB Scan Direction Select (forward)
	GLCDC.OUTSET.BIT.FRQSEL   = 0;  // Pixel Clock Division Control (no division)
	GLCDC.OUTSET.BIT.FORMAT   = 2;  // Output Data Format Select (RGB565)
	GLCDC.OUTSET.BIT.SWAPON   = 0;  // Pixel Order Control (B-G-R)
	GLCDC.OUTSET.BIT.ENDIANON = 0;  // Bit Endian Control (Little endian)
	//
	// Brightness Adjustment
	//
	GLCDC.BRIGHT2.BIT.BRTB = _BRIGHTNESS;  // B
	GLCDC.BRIGHT1.BIT.BRTG = _BRIGHTNESS;  // G
	GLCDC.BRIGHT2.BIT.BRTR = _BRIGHTNESS;  // R
	//
	// Contrast Adjustment Value
	//
	GLCDC.CONTRAST.BIT.CONTB = _CONTRAST;  // B
	GLCDC.CONTRAST.BIT.CONTG = _CONTRAST;  // G
	GLCDC.CONTRAST.BIT.CONTR = _CONTRAST;  // R
	//
	// Enable VPOS ISR
	//
	GLCDC.GR2CLUTINT.BIT.LINE               = YSIZE_PHYS;  // Detecting Scanline Setting
	GLCDC.DTCTEN.BIT.VPOSDTC                = 1;           // Enable detection of specified line notification in graphic 2
	GLCDC.INTEN.BIT.VPOSINTEN               = 1;           // Enable VPOS interrupt request
	ICU.IPR[VECT_ICU_GROUPAL1].BIT.IPR      = 7;           // Interrupt Priority Level
	ICU.IER[VECT_ICU_GROUPAL1 / 8].BIT.IEN1 = 1;           // Interrupt Request Enable
	GLCDC.STCLR.BIT.VPOSCLR                 = 1;           // Clears the STMON.VPOS flag
	ICU.GENAL1.BIT.EN8                      = 1;           // VPOS (line detection)
	//
	// Set function to be called on VSYNC
	//
	R_BSP_InterruptWrite(BSP_INT_SRC_AL1_GLCDC_VPOS, _VSYNC_ISR);
	//
	// Register Reflection
	//
	GLCDC.OUTVEN.BIT.VEN = 0x00000001;  // Output Control Block Register Value Reflection
	GLCDC.BGEN.LONG      = 0x00010101;  // Enable background generation block

	for(i =0;i<256;i++)
	{
	    LCD_X_SETLUTENTRY_INFO * p;
		colorcnf.Color = lcduteconf[i];
		colorcnf.Pos = i;
		p = (LCD_X_SETLUTENTRY_INFO *)&colorcnf;
		_SetLUTEntry(p->Color, p->Pos);
	}
	_DisplayOnOff(1);

//	memset(frame_buffer1, 0, sizeof(frame_buffer1));
	memset(frame_buffer2, 0, sizeof(frame_buffer2));

	return 0;
}

void lcd_close(void)
{
	_DisplayOnOff(0);
	LCD_shutdownController();
}

uint32_t* lcd_get_frame_buffer_pointer(void)
{
	return frame_buffer2;
}

static void _VSYNC_ISR(void * p)
{
	GLCDC.STCLR.BIT.VPOSCLR = 1;                  // Clears the STMON.VPOS flag
	_PendingBuffer = 0;
}

static void _SetLUTEntry(LCD_COLOR Color, uint8_t Pos)
{
	GLCDC.GR2CLUT0[Pos].BIT.R = (Color & 0xFF0000) >> 16;
	GLCDC.GR2CLUT0[Pos].BIT.G = (Color & 0x00FF00) >> 8;
	GLCDC.GR2CLUT0[Pos].BIT.B = (Color & 0x0000FF);
}

static void _DisplayOnOff(int OnOff)
{
	if (OnOff)
	{
		GLCDC.BGEN.LONG   = 0x00010101;  // Enable background generation block
		PORT6.PODR.BIT.B6 = 1;
	}
	else
	{
		GLCDC.BGEN.LONG   = 0x00010100;  // Disable background generation block
		PORT6.PODR.BIT.B6 = 0;
	}
}

static void LCD_shutdownController(void)
{
	R_BSP_InterruptsDisable();
	ICU.GENAL1.BIT.EN8                      = 0;           // VPOS (line detection)
	//	  GLCDC.STCLR.BIT.VPOSCLR                 = 0;           // Clears the STMON.VPOS flag
	ICU.IER[VECT_ICU_GROUPAL1 / 8].BIT.IEN1 = 0;           // Interrupt Request Enable
	ICU.IPR[VECT_ICU_GROUPAL1].BIT.IPR      = 0;           // Interrupt Priority Level
	R_BSP_InterruptsEnable();
}

void my_sw_charput_function(uint8_t data)
{
	R_SIMPLE_GRAPHIC_PutCharacter(data);
}

void my_sw_charget_function(void)
{
	nop();
}

#if SECURE_BOOT
#pragma section
#endif
