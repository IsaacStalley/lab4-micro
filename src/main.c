#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
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

bool usb_com;

// Function to send strings to the established USB COM
static void send_string_over_usart(const char *str) {
    for(int i = 0; str[i] != '\0'; i++) {
        uint16_t data = (uint16_t)str[i]; // Convert character to 16-bit data word
        usart_send_blocking(USART1, data);
    }
}

// Function to setup button and LEDS
static void button_setup(void)
{
	/* Enable GPIOA clock. */
	rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable (RCC_GPIOG) ;
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);
    gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT,GPIO_PUPD_NONE, GPIO13 | GPIO14 );
    rcc_periph_clock_enable(RCC_SYSCFG);
    exti_select_source(EXTI0, GPIOA);
    exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING); // Trigger on falling edge
    exti_enable_request(EXTI0);
    nvic_enable_irq(NVIC_EXTI0_IRQ);
}

// Interrupt function for button
void exti0_isr(void) {
    if (exti_get_flag_status(EXTI0)) {
        usb_com = !usb_com;
        exti_reset_request(EXTI0); // Clear the pending interrupt
    }
}

int main(void)
{   
    usb_com = false;
    int16_t vecs[3];
    char x_str[15];
    char y_str[15];
    char z_str[15];
    char v_str[15];

    // Setups for all peripherals
	clock_setup();
	sdram_init();
	lcd_spi_init();
    mems_init();
    adc_setup();
    usbd_device *usbd_dev = usb_setup();
    usart_setup();
    button_setup();
	gfx_init(lcd_draw_pixel, LCD_WIDTH, LCD_HEIGHT);

    while (1)
    {   
        // Read gyro values
        (void) read_xyz(vecs);
        vecs[0] = vecs[0] / SSF_2000;
        vecs[1] = vecs[1] / SSF_2000;
        vecs[2] = vecs[2] / SSF_2000;
        // Read ACD input on PB0
        uint16_t input_adc0 = read_adc_naiive();

        sprintf(x_str, "X: %d", vecs[0]);
        sprintf(y_str, "Y: %d", vecs[1]);
        sprintf(z_str, "Z: %d", vecs[2]);
        sprintf(v_str, "V_BAT: %d", input_adc0);

        // Display frame drawing
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
        if (usb_com){
            gfx_puts("USB_COM: ON");
        }
        else{
            gfx_puts("USB_COM: OFF");
            gpio_clear(GPIOG, GPIO13);
        }
        // Display show frame
        lcd_show_frame();

        // Battery warning LED
        if (input_adc0 <= 7){
            gpio_set(GPIOG, GPIO14);
        }
        else{
            gpio_clear(GPIOG, GPIO14);
        }

        // Send values over USB if activated
        if (usb_com)
        {
            usbd_poll(usbd_dev);
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
}