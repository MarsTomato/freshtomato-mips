/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>
#include <sys/ioctl.h>
#include <linux_gpio.h>

#include "utils.h"
#include "shutils.h"
#include "shared.h"


const char *led_names[] = {"wlan", "diag", "white", "amber", "dmz", "aoss", "bridge", "usb", "5g"};



static int _gpio_ioctl(int f, int gpioreg, unsigned int mask, unsigned int val)
{
	struct gpio_ioctl gpio;

	gpio.val = val;
	gpio.mask = mask;

	if (ioctl(f, gpioreg, &gpio) < 0) {
		_dprintf("Invalid gpioreg %d\n", gpioreg);
		return -1;
	}
	return (gpio.val);
}

static int _gpio_open()
{
	int f = open("/dev/gpio", O_RDWR);
	if (f < 0)
		_dprintf ("Failed to open /dev/gpio\n");
	return f;
}

int gpio_open(uint32_t mask)
{
	uint32_t bit = 0;
	int i = 0 ;
	int f = _gpio_open();

	if ((f >= 0) && mask) {
		for (i = TOMATO_GPIO_MIN; i <= TOMATO_GPIO_MAX; i++) {
			bit = 1 << i;
			if ((mask & bit) == bit) {
				_gpio_ioctl(f, GPIO_IOC_RESERVE, bit, bit);
				_gpio_ioctl(f, GPIO_IOC_OUTEN, bit, 0);
			}
		}
		close(f);
		f = _gpio_open();
	}
	return f;
}

void gpio_write(uint32_t bit, int en)
{
	int f;

	if ((f = gpio_open(0)) < 0) return;

	_gpio_ioctl(f, GPIO_IOC_RESERVE, bit, bit);
	_gpio_ioctl(f, GPIO_IOC_OUTEN, bit, bit);
	_gpio_ioctl(f, GPIO_IOC_OUT, bit, en ? bit : 0);
	close(f);
}

uint32_t _gpio_read(int f)
{
	uint32_t r;
	r = _gpio_ioctl(f, GPIO_IOC_IN, 0x07FF, 0);
	if (r < 0) r = ~0;
	return r;
}

uint32_t gpio_read(void)
{
	int f;
	uint32_t r;

	if ((f = gpio_open(0)) < 0) return ~0;
	r = _gpio_read(f);
	close(f);
	return r;
}

int nvget_gpio(const char *name, int *gpio, int *inv)
{
	char *p;
	uint32_t n;

	if (((p = nvram_get(name)) != NULL) && (*p)) {
		n = strtoul(p, NULL, 0);
		if ((n & 0xFFFFFF60) == 0) {		/* bin 0110 0000 */
			*gpio = (n & TOMATO_GPIO_MAX);	/* bin 0001 1111 */
			*inv = ((n & 0x80) != 0);	/* bin 1000 0000 */
			/* 0x60 + 0x1F (dec 31) + 0x80 = 0xFF */
			return 1;
		}
	}
	return 0;
}

/* Routine to write to shift register
 * Note that the controls are active low, but input as high = on
 */
void gpio_write_shiftregister(unsigned int led_status, int clk, int data, int max_shifts)
{
	int i;

	gpio_write(1 << data, 1);	/* set data to 1 to start (disable) */
	gpio_write(1 << clk, 0);	/* and clear clock ... */

	for (i = max_shifts; i >= 0; i--) {
		if (led_status & (1 << i))
			gpio_write(1 << data, 0);	/* on, pull low (active low) */
		else
			gpio_write(1 << data, 1);	/* off, pull high (active low) */

		gpio_write(1 << clk, 1);	/* pull high to trigger */
		gpio_write(1 << clk, 0);	/* reset to low -> finish clock cycle*/
	}
}

/* strBits:  convert binary value to string (binary file representation) */
char strConvert[33];
char * strBits(int input, int binarySize)
{

	int i;

	if (binarySize > 0) {
		if (binarySize > 32)
			binarySize = 32;

		for(i = 0; i < binarySize ; i++) {
			if (input & (1 << ((binarySize-1)-i)))
				strConvert[i] = '1';
			else
				strConvert[i] = '0';
		}

		strConvert[binarySize] = '\0';
		return (char *)strConvert;
	} else
		return (char *)NULL;
}

void led_bit(int b, int mode)
{
	FILE *fileExtGPIOstatus;		/* For WNDR4000, keep track of extended bit status (shift register), as cannot read from HW! */
	unsigned int intExtendedLEDStatus;	/* Status of Extended LED's (shift register on WNDR4000) ... and WNDR3700v3, it's the same! */

	if ((mode == LED_ON) || (mode == LED_OFF)) {
		if (b < 16) {
			/* Read bit-mask from file, for tracking / updates (as this process is called clean each LED update, so cannot use static variable!) */
			if ((fileExtGPIOstatus = fopen("/tmp/.ext_led_value", "rb"))) {
				fscanf(fileExtGPIOstatus, "Shift Register Status: 0x%x\n", &intExtendedLEDStatus);
				fclose(fileExtGPIOstatus);
			} else {
				/* Read Error (tracking file) - set all LED's to off */
				intExtendedLEDStatus = 0x00;
			}

			if (mode == LED_ON) {
				/* Bitwise OR, turn corresponding bit on */
				intExtendedLEDStatus |= (1 << b);
			} else {
				/* Bitwise AND, with bitwise inverted shift ... so turn bit off */
				intExtendedLEDStatus &= (~(1 << b));
			}

			/* And write to LEDs (Shift Register) */
			gpio_write_shiftregister(intExtendedLEDStatus, 7, 6, 7);
			/* Write bit-mask to file, for tracking / updates (as this process is called clean each LED update, so cannot use static variable!) */
			if ((fileExtGPIOstatus = fopen("/tmp/.ext_led_value", "wb"))) {
				fprintf(fileExtGPIOstatus, "Shift Register Status: 0x%x\n", intExtendedLEDStatus);
				fprintf(fileExtGPIOstatus, "Shift Register Status: 0b%s\n", strBits(intExtendedLEDStatus, 8));
				fclose(fileExtGPIOstatus);
			}
		}
	}
}

int do_led(int which, int mode)
{
  /*
   * valid GPIO values: 0 to 31 (default active LOW, inverted or active HIGH with -[value])
   * value 255: not known / disabled / not possible
   * value -99: special case for -0 substitute (active HIGH for GPIO 0)
   * value 254: non GPIO LED (special case, to show there is something!)
   */
//				    WLAN  DIAG  WHITE AMBER  DMZ  AOSS  BRIDGE MYST/USB 5G
//				    ----- ----- ----- -----  ---  ----  ------ -----    --
	static int wrt54g[]	= { 255,    1,     2,    3,    7,  255,  255,  255,    255};
	static int wrtsl[]	= { 255,    1,     5,    7,    0,  255,  255,  255,    255};
	static int whrg54[]	= {   2,    7,   255,  255,  255,    6,    1,    3,    255};
	static int wbr2g54[]	= { 255,   -1,   255,  255,  255,   -6,  255,  255,    255};
	static int wzrg54[]	= {   2,    7,   255,  255,  255,    6,  255,  255,    255};
	static int wr850g1[]	= {   7,    3,   255,  255,  255,  255,  255,  255,    255};
	static int wr850g2[]	= {   0,    1,   255,  255,  255,  255,  255,  255,    255};
	static int wtr54gs[]	= {   1,   -1,   255,  255,  255,  255,  255,  255,    255};
	static int dir320[]	= { -99,    1,     4,    3,  255,  255,  255,   -5,    255};
	static int h618b[]	= { 255,   -1,   255,  255,  255,   -5,   -3,   -4,    255};
	static int wl1600gl[]	= {   1,   -5,     0,  255,  255,    2,  255,  255,    255};
	static int wrt310nv1[]	= { 255,    1,     9,    3,  255,  255,  255,  255,    255};
	static int wrt160nv1[]	= { 255,    1,     5,    3,  255,  255,  255,  255,    255};
#ifdef CONFIG_BCMWL5
	static int wnr3500[]	= { 255,  255,    -2,  255,  255,   -1,  255,  255,    255};
	static int wnr2000v2[]	= { 255,  255,   255,  255,  255,   -7,  255,  255,    255};
	static int wndr4000[]	= {   3,    1,     0,    1,  255,    6,  255,    5,      4};
	static int wndr3400[]	= {  -9,   -7,    -3,   -7,  255,  255,  255,    2,    -99}; /* Note: 5 = Switch, 4 = Reset button, 8 = SES button */
	static int wndr3400v3[]	= { -17,  -16,   -14,   14,  255,  -23,  255,  -20,    -18};
	static int f7d[]	= { 255,  255,   255,  255,   12,   13,  255,   14,    255};
	static int wrt160nv3[]	= { 255,    1,     4,    2,  255,  255,  255,  255,    255};
	static int e900[]	= { 255,   -6,     8,  255,  255,  255,  255,  255,    255};
	static int e1000v2[]	= { 255,   -6,     8,    7,  255,  255,  255,  255,    255};
	static int e3200[]	= { 255,   -3,   255,  255,  255,  255,  255,  255,    255};
	static int wrt320n[]	= { 255,    2,     3,    4,  255,  255,  255,  255,    255};
	static int wrt610nv2[]	= { 255,    5,     3,    0,  255,  255,  255,   -7,    255};
	static int e4200[]	= { 255,    3,     5,  255,  255,  255,  255,  255,    255};
	static int rtn10u[]	= { 255,  255,   255,  255,  255,   -7,  255,   -8,    255};
	static int rtn10p[]	= { 255,   -6,   255,  255,  255,   -7,  255,  255,    255};
	static int rtn12a1[]	= { 255,  255,   255,  255,  255,   -2,  255,  225,    255};
	static int rtn12b1[]	= {  -5,  255,     4,  255,  255,  -18,  255,  225,    255};
	static int rtn12c1[]	= {  -4,  255,     5,  255,  255,  -18,  255,  225,    255};
	static int rtn12d1[]	= {  -5,  255,   255,  255,  255,  -18,  255,  225,    255};
	static int rtn15u[]	= {  -1,  255,     3,  255,  255,   -6,    4,   -9,    255};
	static int rtn53[]	= {   0,  -17,   255,  255,  255,  255,  255,  255,    255};
	static int l600n[]	= { 255,  255,   255,  255,  255,   -7,  255,   -8,    255};
	static int dir620c1[]	= {  -6,   -8,   255,  255,  255,   -7,  255,  255,    255};
	static int rtn66u[]	= { 255,  -12,   255,  255,  255,  255,  255,   15,     13};
	static int w1800r[]	= { 255,  -13,   255,  255,  255,  255,  255,  -12,     -5};
	static int d1800h[]	= { -12,  -13,     8,  255,  255,  -10,  255,   15,     11};
	static int tdn6[]	= { 255,   -6,     8,  255,  255,  255,  255,  255,    255};
	static int tdn60[]	= { 255,   -6,   255,  255,  255,  255,  255,    9,    255};
	static int dir865l[]	= { 255,  -99,     2,    1,  255,    3,  255,  255,    255};
	static int r6300v1[]	= {  11,    3,   255,  255,  255,  255,  255,    8,     11};
	static int wndr4500[]	= {   9,    3,     2,    3,  255,  255,  255,   14,     11};
#endif
//				    ----- ----- ----- -----  ---  ----  ------ -----    --
//				    WLAN  DIAG  WHITE AMBER  DMZ  AOSS  BRIDGE MYST/USB 5G

	char s[16];
	int n;
	int b = 255, c = 255;
	int ret = 255;
	static int model = 0; /* initialize with 0 / MODEL_UNKNOWN */

	if ((which < 0) || (which >= LED_COUNT)) return ret;

	if (model == 0) { /* router model unknown OR detect router model for the first time at function do_led(). */
		/* get router model */
		model = get_model();
	}

	switch (nvram_match("led_override", "1") ? MODEL_UNKNOWN : model) {
	case MODEL_WRT54G:
		if (check_hw_type() == HW_BCM4702) {
			/* G v1.x */
			if ((which != LED_DIAG) && (which != LED_DMZ)) return ret;
			b = (which == LED_DMZ) ? 1 : 4;
			if (mode != LED_PROBE) {
				if (f_read_string("/proc/sys/diag", s, sizeof(s)) > 0) {
					n = atoi(s);
					sprintf(s, "%u", mode ? (n | b) : (n & ~b));
					f_write_string("/proc/sys/diag", s, 0, 0);
				}
			}
			return b;
		}
		switch (which) {
		case LED_AMBER:
		case LED_WHITE:
			if (!supports(SUP_WHAM_LED)) return ret;
			break;
		}
		b = wrt54g[which];
		break;
	case MODEL_WTR54GS:
		b = wtr54gs[which];
		break;
	case MODEL_WRTSL54GS:
		b = wrtsl[which];
		break;
	case MODEL_WHRG54S:
	case MODEL_WHRHPG54:
	case MODEL_WHRG125:
		b = whrg54[which];
		break;
	case MODEL_WZRG54:
	case MODEL_WZRHPG54:
	case MODEL_WZRRSG54:
	case MODEL_WZRRSG54HP:
	case MODEL_WVRG54NF:
	case MODEL_WHR2A54G54:
	case MODEL_WHR3AG54:
	case MODEL_WZRG108:
		b = wzrg54[which];
		break;
/*
	case MODEL_WHR2A54G54:
		if (which != LED_DIAG) return ret;
		b = 7;
		break;
*/
	case MODEL_WBRG54:
		if (which != LED_DIAG) return ret;
		b = 7;
		break;
	case MODEL_WBR2G54:
		b = wbr2g54[which];
		break;
	case MODEL_WR850GV1:
		b = wr850g1[which];
		break;
	case MODEL_WR850GV2:
	case MODEL_WR100:
		b = wr850g2[which];
		break;
	case MODEL_WL500GP:
		if (which != LED_DIAG) return ret;
		b = -1;	/* power light */
		break;
	case MODEL_WL500W:
		if (which != LED_DIAG) return ret;
		b = -5;	/* power light */
		break;
	case MODEL_DIR320:
		b = dir320[which];
		break;
	case MODEL_H618B:
		b = h618b[which];
		break;
	case MODEL_WL1600GL:
		b = wl1600gl[which];
		break;
	case MODEL_WL500GPv2:
	case MODEL_WL500GD:
	case MODEL_WL520GU:
	case MODEL_WL330GE:
		if (which != LED_DIAG) return ret;
		b = -99;	/* Invert power light as diag indicator */
		break;
#ifdef CONFIG_BCMWL5
	case MODEL_RTN10:
	case MODEL_RTN16:
		if (which != LED_DIAG) return ret;
		b = -1;	/* power light */
		break;
	case MODEL_RTN10U:
		b = rtn10u[which];
		break;
	case MODEL_RTN10P:
		b = rtn10p[which];
		break;
	case MODEL_RTN12A1:
		b = rtn12a1[which];
		break;
	case MODEL_RTN12B1:
	case MODEL_RTN12HP:
		b = rtn12b1[which];
		break;
	case MODEL_RTN12C1:
		b = rtn12c1[which];
		break;
	case MODEL_RTN12D1:
	case MODEL_RTN12VP:
		b = rtn12d1[which];
		break;
	case MODEL_RTN15U:
		b = rtn15u[which];
		break;
	case MODEL_RTN53:
	case MODEL_RTN53A1:
		b = rtn53[which];
		break;
	case MODEL_RTN66U:
		b = rtn66u[which];
		break;
	case MODEL_DIR865L:
		b = dir865l[which];
		break;
	case MODEL_W1800R:
	case MODEL_TDN80:
		b = w1800r[which];
		break;
	case MODEL_D1800H:
		if (which == LED_DIAG) {
			/* power led gpio: 0x02 - white, 0x13 - red */
			b = (mode) ? 13 : 2;
			c = (mode) ? 2 : 13;
		} else
			b = d1800h[which];
		break;
	case MODEL_WNR3500L:
	case MODEL_WNR3500LV2:
		if (which == LED_DIAG) {
			b = 3; /* gpio 3 actice HIGH AND gpio 7 active LOW  ==> result: Power LED on green; for amber --> inverted */
			c = 7;
		} else
			b = wnr3500[which];
		break;
	case MODEL_WNDR4500:
	case MODEL_WNDR4500V2:
		if (which == LED_DIAG) {
			/* power led gpio: 0x102 - green, 0x103 - amber */
			b = (mode) ? 3 : -2;
			c = (mode) ? -2 : 3;
		} else {
			b = wndr4500[which];
		}
		break;
	case MODEL_R6300V1:
		b = r6300v1[which];
		break;
	case MODEL_WNR2000v2:
		if (which == LED_DIAG) {
			/* power led gpio: 0x01 - green, 0x02 - amber */
			b = (mode) ? 2 : 1;
			c = (mode) ? 1 : 2;
		} else
			b = wnr2000v2[which];
		break;
	case MODEL_WNDR4000:
	case MODEL_WNDR3700v3:
		/* Special Case, shift register control ... so write accordingly. */
		b = wndr4000[which];
		led_bit(b, mode);
		return b;
		break;
	case MODEL_WNDR3400:
	case MODEL_WNDR3400v2:
		b = wndr3400[which];
		break;
	case MODEL_WNDR3400v3:
		b = wndr3400v3[which];
		break;
	case MODEL_F7D3301:
	case MODEL_F7D3302:
	case MODEL_F7D4301:
	case MODEL_F7D4302:
	case MODEL_F5D8235v3:
		if (which == LED_DIAG) {
			/* power led gpio: 10 - green, 11 - red */
			b = (mode) ? 11 : -10;
			c = (mode) ? -10 : 11;
		} else
			b = f7d[which];
		break;
	case MODEL_E1000v2:
		b = e1000v2[which];
		break;
	case MODEL_E900:
	case MODEL_E1500:
	case MODEL_E1550:
	case MODEL_E2500:
		b = e900[which];
		break;
	case MODEL_E3200:
		b = e3200[which];
		break;
	case MODEL_WRT160Nv3:
		b = wrt160nv3[which];
		break;
	case MODEL_WRT320N:
		b = wrt320n[which];
		break;
	case MODEL_WRT610Nv2:
		b = wrt610nv2[which];
		break;
	case MODEL_E4200:
		b = e4200[which];
		break;
	case MODEL_L600N:
		b = l600n[which];
		break;
	case MODEL_DIR620C1:
		b = dir620c1[which];
	case MODEL_TDN60:
		b = tdn60[which];
	case MODEL_TDN6:
		b = tdn6[which];
		break;
#endif
/*
	case MODEL_RT390W:
		break;
*/
	case MODEL_MN700:
		if (which != LED_DIAG) return ret;
		b = 6;
		break;
	case MODEL_WLA2G54L:
		if (which != LED_DIAG) return ret;
		b = 1;
		break;
	case MODEL_WRT300N:
		if (which != LED_DIAG) return ret;
		b = 1;
		break;
	case MODEL_WRT310Nv1:
		b = wrt310nv1[which];
		break;
	case MODEL_WRT160Nv1:
		b = wrt160nv1[which];
		break;
	default:
		sprintf(s, "led_%s", led_names[which]);
		if (nvget_gpio(s, &b, &n)) {
			if ((mode != LED_PROBE) && (n)) mode = !mode;
			ret = (n) ? b : ((b) ? -b : -99);
			goto SET;
		}
		return ret;
	}

	ret = b;
	if (b < TOMATO_GPIO_MIN) {
		if (b == -99)
			b = TOMATO_GPIO_MIN;	/* -0 substitute */
		else
			b = -b;
	}
	else if (mode != LED_PROBE) {
		mode = !mode;
	}

SET:
	if (b <= TOMATO_GPIO_MAX) {
		if (mode != LED_PROBE) {
			gpio_write(1 << b, mode);

			if (c < TOMATO_GPIO_MIN) {
				if (c == -99)
					c = TOMATO_GPIO_MIN;
				else
					c = -c;
			}
			else
				mode = !mode;

			if (c <= TOMATO_GPIO_MAX) gpio_write(1 << c, mode);
		}
	}

	return ret;
}
