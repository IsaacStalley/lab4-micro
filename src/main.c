#include <stdio.h>
#include <stdint.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include "clock.h"
#include "sdram.h"
#include "lcd-spi.h"
#include "gfx.h"
#include "spi-mems.h"

int16_t twosCompToDec(uint16_t val)
{
    return -((~val + 1) & 0xFFFF);
}


int main(void)
{   
    int16_t vecs[3];
	int16_t tmp, status;
    char tmp_str[50];
    char x_str[50];
    char y_str[50];
    char z_str[50];

	clock_setup();
	sdram_init();
	lcd_spi_init();
    mems_init();

	gfx_init(lcd_draw_pixel, LCD_WIDTH, LCD_HEIGHT);

    while (1)
    {
        tmp = (int) read_reg(0x26);
        tmp = twosCompToDec(tmp);
        status = read_xyz(vecs);
        vecs[0] = twosCompToDec(vecs[0]) / SSF_2000;
        vecs[1] = twosCompToDec(vecs[1]) / SSF_2000;
        vecs[2] = twosCompToDec(vecs[2]) / SSF_2000;

        sprintf(tmp_str, "Temp: %d", tmp);
        sprintf(x_str, "X: %d", vecs[0]);
        sprintf(y_str, "Y: %d", vecs[1]);
        sprintf(z_str, "Z: %d", vecs[2]);

        gfx_fillScreen(LCD_GREY);
        gfx_setTextSize(2);
        gfx_setCursor(15, 25);
        gfx_puts(tmp_str);
        gfx_setCursor(15, 45);
        gfx_puts(x_str);
        gfx_setCursor(15, 65);
        gfx_puts(y_str);
        gfx_setCursor(15, 85);
        gfx_puts(z_str);
        lcd_show_frame();

        msleep(1000);
    }
    
}