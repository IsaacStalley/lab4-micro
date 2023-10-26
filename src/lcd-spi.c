#include <stdint.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include "clock.h"
#include "sdram.h"
#include "lcd-spi.h"

uint16_t *forming_frame;
uint16_t *finished_frame;

void lcd_draw_pixel(int x, int y, uint16_t color)
{
	*(forming_frame + x + y * LCD_WIDTH) = color;
}

struct tft_command {
	uint16_t delay;		/* If you need a delay after */
	uint8_t cmd;		/* command to send */
	uint8_t n_args;		/* How many arguments it has */
};

static const struct tft_command  initialization[] = {
	{   0, 0xb1, 2 },	/* 0x00, 0x1B, */
	{   0, 0xb6, 2 },	/* 0x0a, 0xa2, */
	{   0, 0xc0, 1 },	/* 0x10, */
	{   0, 0xc1, 1 },	/* 0x10, */
	{   0, 0xc5, 2 },	/* 0x45, 0x15, */
	{   0, 0xc7, 1 },	/* 0x90, */
	{   0, 0x36, 1 },	/* 0xc8, */
	{   0, 0xb0, 1 },	/* 0xc2, */
	{   0, 0x3a, 1 },	/* 0x55 **added, pixel format 16 bpp */
	{   0, 0xb6, 4 },	/* 0x0a, 0xa7, 0x27, 0x04, */
	{   0, 0x2A, 4 },	/* 0x00, 0x00, 0x00, 0xef, */
	{   0, 0x2B, 4 },	/* 0x00, 0x00, 0x01, 0x3f, */
	{   0, 0xf6, 3 },	/* 0x01, 0x00, 0x06, */
	{ 200, 0x2c, 0 },
	{   0, 0x26, 1 },	/* 0x01, */
	{   0, 0xe0, 15},	/* 0x0F, 0x29, 0x24, 0x0C, 0x0E, */
				/* 0x09, 0x4E, 0x78, 0x3C, 0x09, */
				/* 0x13, 0x05, 0x17, 0x11, 0x00, */
	{   0, 0xe1, 15},	/* 0x00, 0x16, 0x1B, 0x04, 0x11, */
				/* 0x07, 0x31, 0x33, 0x42, 0x05, */
				/* 0x0C, 0x0A, 0x28, 0x2F, 0x0F, */
	{ 200, 0x11, 0 },
	{   0, 0x29, 0 },
	{   0,    0, 0 }	/* cmd == 0 indicates last command */
};

const uint8_t cmd_args[] = {
	0x00, 0x1B, 0x0a, 0xa2, 0x10,
	0x10, 0x45, 0x15, 0x90, 0x08,
	0xc2, 0x55, 0x0a, 0xa7, 0x27,
	0x04, 0x00, 0x00, 0x00, 0xef,
	0x00, 0x00, 0x01, 0x3f, 0x01,
	0x00, 0x00, 0x01, 0x0F, 0x29,
	0x24, 0x0C, 0x0E, 0x09, 0x4E,
	0x78, 0x3C, 0x09, 0x13, 0x05,
	0x17, 0x11, 0x00, 0x00, 0x16,
	0x1B, 0x04, 0x11, 0x07, 0x31,
	0x33, 0x42, 0x05, 0x0C, 0x0A,
	0x28, 0x2F, 0x0F,
};

void lcd_command(uint8_t cmd, int delay, int n_args, const uint8_t *args)
{
	gpio_clear(GPIOC, GPIO2);	/* Select the LCD */
	(void) spi_xfer(LCD_SPI, cmd);
	if (n_args) {
		gpio_set(GPIOD, GPIO13);	/* Set the D/CX pin */
		for (int i = 0; i < n_args; i++) {
			(void) spi_xfer(LCD_SPI, *(args+i));
		}
	}
	gpio_set(GPIOC, GPIO2);		/* Turn off chip select */
	gpio_clear(GPIOD, GPIO13);	/* always reset D/CX */
	if (delay) {
		msleep(delay);		/* wait, if called for */
	}
}

void initialize_display(const struct tft_command cmds[])
{
	int i = 0;
	int arg_offset = 0;

	while (cmds[i].cmd) {

		lcd_command(cmds[i].cmd, cmds[i].delay, cmds[i].n_args,
			&cmd_args[arg_offset]);
		arg_offset += cmds[i].n_args;
		i++;
	}
}

void lcd_show_frame(void)
{
	uint16_t *temp_frame;
	uint8_t size[4];

	temp_frame = finished_frame;
	finished_frame = forming_frame;
	forming_frame = temp_frame;

	size[0] = 0;
	size[1] = 0;
	size[2] = (LCD_WIDTH >> 8) & 0xff;
	size[3] = (LCD_WIDTH) & 0xff;
	lcd_command(0x2A, 0, 4, (const uint8_t *)&size[0]);
	size[0] = 0;
	size[1] = 0;
	size[2] = (LCD_HEIGHT >> 8) & 0xff;
	size[3] = LCD_HEIGHT & 0xff;
	lcd_command(0x2B, 0, 4, (const uint8_t *)&size[0]);
	lcd_command(0x2C, 0, FRAME_SIZE_BYTES, (const uint8_t *)finished_frame);
}

void lcd_spi_init(void)
{

	rcc_periph_clock_enable(RCC_GPIOC | RCC_GPIOD | RCC_GPIOF);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO2);
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7 | GPIO9);
	gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO9);

	forming_frame = (uint16_t *)(SDRAM_BASE_ADDRESS);
	finished_frame = forming_frame + (LCD_WIDTH * LCD_HEIGHT);

	rcc_periph_clock_enable(RCC_SPI5);
	spi_init_master(LCD_SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_4,
					SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
					SPI_CR1_CPHA_CLK_TRANSITION_1,
					SPI_CR1_DFF_8BIT,
					SPI_CR1_MSBFIRST);
	spi_enable_ss_output(LCD_SPI);
	spi_enable(LCD_SPI);

	initialize_display(initialization);

}