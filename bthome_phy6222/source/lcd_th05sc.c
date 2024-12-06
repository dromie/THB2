/*
 * lcd_th05.c
 *
 *  Created on: 23 янв. 2024 г.
 *      Author: dromie, Shestoperd
 */

// Debug command: 60000000000100        (7)
// Debug command: 60000000000200        (6)
// Debug command: 60000000000400        (3+4 palcika )
// Debug command: 60000000000800        (felso vonal)
// Debug command: 60000000000f00        3
// Debug command: 60000000001000         /(3+4)
// Debug command: 60000000002000          (5+7)
// Debug command: 60000000004000               lb
// Debug command: 60000000008000            kg
// Debug command: 60000000010000            	  st
// Debug command: 60000000020000     (6)      	  
// Debug command: 60000000040000     (3)      	  
// Debug command: 60000000080000     (1)      	  
// Debug command: 60000000100000     (7)      	  
// Debug command: 60000000200000     (5)      	  
// Debug command: 60000000400000     (4)      	  
// Debug command: 60000000800000     (2)
// Debug command: 60000001000000    .
// 02 - 08 (6,3,1,7,5,4,2)
// Debug command: 60000100000000  :
// 02 - 08 (6,3,1,7,5,4,2)
// Debug command: 60010000000000  <NOTHING>
// 02 - 08 (6,3,1,7,5,4,2)
// LCD: 60



#include <string.h>
#include "types.h"
#include "config.h"
#if (DEV_SERVICES & SERVICE_SCREEN)  && (DEVICE == DEVICE_TH05SC)
#include "OSAL.h"
#include "gpio.h"
#include "clock.h"
#include "rom_sym_def.h"
#include "dev_i2c.h"
#include "sensors.h"
#include "lcd.h"
#include "thb2_peripheral.h"

#define LCD_I2C_ADDR	0x3E
#define I2C_WAIT_ms		1

dev_i2c_t i2c_dev1 = {
		//.pi2cdev = AP_I2C1,
		.scl = I2C_LCD_SCL,
		.sda = I2C_LCD_SDA,
		.speed = I2C_100KHZ,
		.i2c_num = 0
};

uint8_t lcd_i2c_addr; // = 0x3E

/*FS(ABCD)			  (08)
					--------      
					|      |
			(80)	|      |	(04)
					| (40) |
					 ------		 
					|      |
			(20)	|	   |	(02)
					|      |
					 ------
					   (10)

*/

/*
0 = 08 + 04 + 02 + 10 + 20 + 80 = 0xBE = 0b10111110
1 = 04 + 02 = 0x06 = 0b00000110
2 = 08 + 04 + 40 + 20 + 10 = 0x7C = 0b01111100
3 = 08 + 04 + 40 + 02 + 10 = 0x5E = 0b01011110
4 = 80 + 40 + 04 + 02 = 0xC6 = 0b11000110
5 = 08 + 80 + 40 + 02 + 10 = 0xDA = 0b11011010
6 = 08 + 80 + 40 + 20 + 02 + 10 = 0xFA = 0b11111010
7 = 08 + 04 + 02 = 0x0E = 0b00001110
8 = 08 + 04 + 40 + 20 + 10 + 80 = 0xFE = 0b11111110
9 = 08 + 04 + 40 + 02 + 10 + 80 = 0xDE = 0b11011110
A = 08 + 04 + 40 + 20 + 02 + 80 = 0xEE = 0b11101110
b = 80 + 40 + 20 + 02 + 10 = 0xF2 = 0b11110010
C = 08 + 80 + 20 + 10 = 0xB8 = 0b10111000
d = 04 + 40 + 20 + 02 + 10 = 0x76 = 0b01110110
E = 08 + 80 + 40 + 20 + 10 = 0xF8 = 0b11111000
F = 08 + 80 + 40 + 20 = 0xE8 = 0b11101000
c = 40 + 20 + 10 = 0x70 = 0b01110000
o = 40 + 20 + 01 + 10 = 0b01110011
fok = 80 + 08 + 40 + 04 = 0b11001100
L = 80 + 20 + 10 = 0b10110000
H = 80 + 40 + 04 + 20 + 02 = 0b11100110
h = 80 + 20 + 40 + 02 = 0b11100010
P = 80 + 08 + 40 + 04 + 20 = 0b11101100
*/

/*3/8	(h)			  (8)
					--------      
					|      |
			(XX)	|      |	(4)
					| (4)  |
					 ------		 
					|      |
			(XX)	|	   |	(2)
					|      |
					 ------
					   (1)

*/
/*3/8	(GI)		  (08)
					--------      
					|      |
			(02)	|      |	(10)
					| (10) |
					 ------		 
					|      |
			(20)	|	   |	(04)
					|      |
					 ------
					   (20)

*/


// AB:C.D X/Y kg lb st
// AaBbCcDdGhI0
// b0=:
// c0=.
// d0=st
// g8=kg
// g4=lb


/* 0,1,2,3,4,5,6,7,8,9,A,b,C,d,E,F*/
const uint8_t display_numbers[] = {
		// 76543210 
		0b010111110, // 0
		0b000000110, // 1
		0b001111100, // 2
		0b001011110, // 3
		0b011000110, // 4
		0b011011010, // 5
		0b011111010, // 6
		0b000001110, // 7
		0b011111110, // 8
		0b011011110, // 9
		0b011101110, // A
		0b011110010, // b
		0b010111000, // C
		0b001110110, // d
		0b011111000, // E
		0b011101000  // F
};
// 					  76543210        
#define LCD_SYM_m  0b001000000 // "-"
#define LCD_SYM_b  0b011110010 // "b"
#define LCD_SYM_H  0b011100110 // "H"
#define LCD_SYM_h  0b011100010 // "h"
#define LCD_SYM_i  0b000000010 // "i"
#define LCD_SYM_L  0b010110000 // "L"
#define LCD_SYM_o  0b001110011 // "o"
#define LCD_SYM_t  0b011100010 // "t"
#define LCD_SYM_0  0b010111110 // "0"
#define LCD_SYM_A  0b011101110 // "A"
#define LCD_SYM_a  0b011110110 // "a"
#define LCD_SYM_P  0b011101100 // "P"

uint8_t display_buff[LCD_BUF_SIZE] = {
		0, LCD_SYM_o, LCD_SYM_o, 0
};
uint8_t display_out_buff[LCD_BUF_SIZE+1] = { 8, 0 };

const uint8_t lcd_init_cmd[]	=	{
		// LCD controller initialize:
		0xea, // Set IC Operation(ICSET): Software Reset, Internal oscillator circuit
		0xd8, // Mode Set (MODE SET): Display enable, 1/3 Bias, power saving
		0xbc, // Display control (DISCTL): Power save mode 3, FRAME flip, Power save mode 1
		0x80, // load data pointer
		0xf0, // blink control off,  0xf2 - blink
		0xfc, // All pixel control (APCTL): Normal
		0x60,
		0x00,0x00,000,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

/* 0x0 = "  "
 * 0x1 = "°Г"
 * 0x2 = " _"
 * 0x3 = "°C"
 * 0x4 = " -"
 * 0x5 = "°F"
 * 0x6 = " ="
 * 0x7 = "°E" */
void show_temp_symbol(LCD_TEMP_SYMBOLS symbol) {
	display_buff[2] &= ~BIT(3);
	display_buff[3] &= ~(BIT(6) | BIT(7));
	if(symbol & 1)
		display_buff[3] |= BIT(6);
	if(symbol & 2)
		display_buff[2] |= BIT(3);
	if(symbol & 4)
		display_buff[3] |= BIT(7);
}

/* 0 = "     " off,
 * 1 = " ^_^ " happy
 * 2 = " -^- " sad
 * 3 = " ooo "
 * 4 = "(   )"
 * 5 = "(^_^)" happy
 * 6 = "(-^-)" sad
 * 7 = "(ooo)" */
void show_smiley(LCD_SMILEY_SYMBOLS symbol) {
	display_buff[3] &= ~(BIT(0) | BIT(1) | BIT(4));
	if(symbol & 1)
		display_buff[3] |= BIT(0);
	if(symbol & 2)
		display_buff[3] |= BIT(1);
	if(symbol & 4)
		display_buff[3] |= BIT(4);
}

void show_ble_symbol(bool state) {
	if (state)
		display_buff[4] |= BIT(3);
	else
		display_buff[4] &= ~BIT(3);
}

void show_battery_symbol(bool state) {
	if (state)
		display_buff[3] |= BIT(5);
	else
		display_buff[3] &= ~BIT(5);
}

void show_big_number_x10(int16_t number) {
	if (number > 9999) {
   		display_buff[0] = LCD_SYM_H; // "H"
   		display_buff[1] = LCD_SYM_i; // "i"
	} else if (number < -995) {
   		display_buff[0] = LCD_SYM_L; // "L"
   		display_buff[1] = LCD_SYM_o; // "o"
	} else {
		display_buff[0] = 0;
		display_buff[1] = 0;
		/* number: -995..19995 */
		if (number > 1995 || number < -95) {
			display_buff[1] = 0; // no point, show: -99..1999
			if (number < 0){
				number = -number;
				display_buff[0] = LCD_SYM_m
			}
			number = (number + 5) / 10; // round(div 10)
		} else { // show: -9.9..199.9
			display_buff[3] = BIT(1); // point,
			if (number < 0){
				number = -number;
				display_buff[0] = LCD_SYM_m; // "-"
			}
		}
		/* number: -99..1999 */
		if (number > 999) display_buff[0] |= BIT(3); // "1" 1000..1999
		if (number > 99) display_buff[0] |= display_numbers[number / 100 % 10];
		if (number > 9) display_buff[1] |= display_numbers[number / 10 % 10];
		else display_buff[1] |= LCD_SYM_0; // "0"
	    display_buff[2] |= display_numbers[number %10];
	}
}

/* -9 .. 99 */
void show_small_number(int16_t number, bool percent) {

	display_buff[4] &= BIT(3); // connect
	display_buff[5] = percent? BIT(3) : 0;
	if (number > 99) {
		display_buff[4] |= LCD_SYM_H; // "H"
		display_buff[5] |= LCD_SYM_i; // "i"
	} else if (number < -9) {
		display_buff[4] |= LCD_SYM_L; // "L"
		display_buff[5] |= LCD_SYM_o; // "o"
	} else {
		if (number < 0) {
			number = -number;
			display_buff[4] = BIT(2); // "-"
		}
		if (number > 9) display_buff[4] |= display_numbers[number / 10 % 10];
		display_buff[5] |= display_numbers[number %10];
	}
}

void lcd_show_version(void) {
#if OTA_TYPE
	display_buff[0] = LCD_SYM_b;
	display_buff[1] = LCD_SYM_o;
#else
	display_buff[0] = LCD_SYM_A;
	display_buff[1] = LCD_SYM_P;
#endif
	display_buff[4] &= BIT(1); // bat
	display_buff[5] &= BIT(8); // connect
	display_buff[3] |= display_numbers[(APP_VERSION >> 4) & 0x0f];
	display_buff[4] = display_numbers[APP_VERSION & 0x0f];
	update_lcd();
}

void chow_clock(void) {
	uint32_t tmp = clkt.utc_time_sec / 60;
	uint32_t min = tmp % 60;
	uint32_t hrs = (tmp / 60) % 24;
	display_buff[0] = 0;
	display_buff[1] = display_numbers[(hrs / 10) % 10];
	display_buff[2] = display_numbers[hrs % 10];
	display_buff[3] &= BIT(5); // bat
	display_buff[4] &= BIT(3); // connect
	display_buff[4] |= display_numbers[(min / 10) % 10];
	display_buff[5] = display_numbers[min % 10];
	update_lcd();
}

static void chow_measure(void) {
#if (DEV_SERVICES & SERVICE_THS)
	if(cfg.flg & FLG_SHOW_TF) {
		show_big_number_x10(((int32_t)((int32_t)measured_data.temp * 9)/ 50) + 320); // convert C to F
		show_temp_symbol(LCD_TSYMBOL_F); // "°F"
	} else {
		show_big_number_x10((measured_data.temp + 5)/10);
		show_temp_symbol(LCD_TSYMBOL_C);
	}
	int16_t h = (measured_data.humi + 50)/100;
	if(h > 99)
		h = 99;
	show_small_number(h, true);
	show_battery_symbol(measured_data.battery < 20);
#if (OTA_TYPE == OTA_TYPE_APP)
	if(cfg.flg & FLG_SHOW_SMILEY) {
#if (DEV_SERVICES & SERVICE_TH_TRG)
		if(cfg.flg & FLG_SHOW_TRG) {
			if(measured_data.flg.comfort) {
				if(measured_data.flg.trg_on)
					show_smiley(LD_SSYMBOL_HAPPY);
				else
					show_smiley(LD_SSYMBOL__HAPPY);
			} else {
				if(measured_data.flg.trg_on)
					show_smiley(LD_SSYMBOL_SAD);
				else
					show_smiley(LD_SSYMBOL__SAD);
			}
		} else
#endif // SERVICE_TH_TRG
		{
			if(measured_data.flg.comfort)
				show_smiley(LD_SSYMBOL_HAPPY);
			else
				show_smiley(LD_SSYMBOL_SAD);
		}
#if (DEV_SERVICES & SERVICE_TH_TRG)
	} else if(cfg.flg & FLG_SHOW_TRG) {
		if(measured_data.flg.trg_on)
			show_smiley(LD_SSYMBOL_ALL);
		else
			show_smiley(LD_SSYMBOL_OFF);
	} else
#endif // SERVICE_TH_TRG
#endif // OTA_TYPE
		show_smiley(LD_SSYMBOL_OFF);
#else
	show_big_number_x10(measured_data.battery_mv/100);
	show_small_number((measured_data.battery > 99)? 99 : measured_data.battery, true);
	show_battery_symbol(1);
	show_smiley(LD_SSYMBOL_OFF);
#endif // SERVICE_THS
	show_ble_symbol(gapRole_state == GAPROLE_CONNECTED);
	update_lcd();
}

/* flg != 0 -> chow_measure */
void chow_lcd(int flg) {
	if(wrk.lcd_ext_chow) // показ TH/Clock отключен
		return;
#if OTA_TYPE == OTA_TYPE_BOOT
	if(flg)
		chow_measure();
#else
	if(cfg.flg & FLG_SHOW_TIME) {
		if(wrk.lcd_count++ & 1)
			chow_clock();
		else
			chow_measure();
	} else if(flg) {
		chow_measure();
	}
#endif
}

void send_to_lcd(uint8_t *pbuf, int len) {
	if (lcd_i2c_addr) {
		init_i2c(&i2c_dev1);
		send_i2c_buf(&i2c_dev1, lcd_i2c_addr, pbuf, len);
		deinit_i2c(&i2c_dev1);
	}
}


void update_lcd(void) {
#if (OTA_TYPE == OTA_TYPE_APP)
	if(lcd_i2c_addr == 0 || (cfg.flg & FLG_DISPLAY_OFF) != 0)
		return;
#endif
	if(memcmp(&display_out_buff[1], display_buff, sizeof(display_buff))) {
		memcpy(&display_out_buff[1], display_buff, sizeof(display_buff));
		send_to_lcd(display_out_buff, sizeof(display_out_buff));
	}
}

void init_lcd(void) {
	i2c_dev1.speed = I2C_100KHZ;
#if (OTA_TYPE == OTA_TYPE_APP)
	if(hal_gpio_read(GPIO_LPWR) == 0) {
		hal_gpio_write(GPIO_LPWR, 1);
		WaitMs(2);
	}
#endif
	init_i2c(&i2c_dev1);
	if(!send_i2c_buf(&i2c_dev1, LCD_I2C_ADDR, (uint8_t *) lcd_init_cmd, sizeof(lcd_init_cmd))) {
#if (OTA_TYPE == OTA_TYPE_APP)
		//display_out_buff[0] = 8;
		if(cfg.flg & FLG_DISPLAY_OFF) {
			send_i2c_byte(&i2c_dev1, LCD_I2C_ADDR, 0xd0); // Mode Set (MODE SET): Display disable, 1/3 Bias, power saving
			deinit_i2c(&i2c_dev1);
			hal_gpio_write(GPIO_LPWR, 0);
			return;
		}
#endif
		lcd_i2c_addr = LCD_I2C_ADDR;
	} else
		lcd_i2c_addr = 0;
	i2c_dev1.speed = I2C_400KHZ;
	deinit_i2c(&i2c_dev1);
}

/****************************************************/
#endif // (DEV_SERVICES & SERVICE_SCREEN)

