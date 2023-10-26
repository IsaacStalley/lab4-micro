#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/usart.h>
#include "clock.h"
#include "sdram.h"
#include "lcd-spi.h"
#include "gfx.h"
#include "spi-mems.h"
#include "adc.h"
#include "usb-com.h"
#include "usart.h"

void send_string_over_usart(const char *str) {
    for(int i = 0; str[i] != '\0'; i++) {
        uint16_t data = (uint16_t)str[i]; // Convert character to 16-bit data word
        usart_send_blocking(USART1, data);
    }
}

int main(void)
{   
    int16_t vecs[3];
    char x_str[15];
    char y_str[15];
    char z_str[15];
    char v_str[15];

	clock_setup();
	sdram_init();
	lcd_spi_init();
    mems_init();
    adc_setup();
    usbd_device *usbd_dev = usb_setup();
    usart_setup();

	gfx_init(lcd_draw_pixel, LCD_WIDTH, LCD_HEIGHT);

    while (1)
    {   
        (void) read_xyz(vecs);
        vecs[0] = vecs[0] / SSF_2000;
        vecs[1] = vecs[1] / SSF_2000;
        vecs[2] = vecs[2] / SSF_2000;
        uint16_t input_adc0 = read_adc_naiive();

        sprintf(x_str, "X: %d", vecs[0]);
        sprintf(y_str, "Y: %d", vecs[1]);
        sprintf(z_str, "Z: %d", vecs[2]);
        sprintf(v_str, "V_BAT: %d", input_adc0);

        gfx_fillRoundRect(0, 0, LCD_WIDTH, LCD_HEIGHT, 0, LCD_WHITE);
        gfx_setTextSize(2);
        gfx_setCursor(15, 25);
        gfx_puts("SISMOGRAFO");
        gfx_setCursor(15, 55);
        gfx_puts("B97756");
        gfx_setTextSize(3);
        gfx_setCursor(15, 95);
        gfx_puts(x_str);
        gfx_setCursor(15, 135);
        gfx_puts(y_str);
        gfx_setCursor(15, 175);
        gfx_puts(z_str);
        gfx_setCursor(15, 240);
        gfx_setTextSize(2);
        gfx_puts(v_str);
        gfx_setCursor(15, 270);
        gfx_puts("USB_COM: OFF");
        lcd_show_frame();
        usbd_poll(usbd_dev);
        // Ensure the endpoint is ready for transmission

        gpio_toggle(GPIOG, GPIO13);	/* LED on/off */
		send_string_over_usart(x_str);
        usart_send_blocking(USART1, ',');
        send_string_over_usart(y_str);
        usart_send_blocking(USART1, ',');
        send_string_over_usart(z_str);
        usart_send_blocking(USART1, ',');
        send_string_over_usart(v_str);
        usart_send_blocking(USART1, ',');
        usart_send_blocking(USART1, '\r');
		usart_send_blocking(USART1, '\n');
    
    }
    
}