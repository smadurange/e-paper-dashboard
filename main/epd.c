#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>

#include "epd.h"

#define EPD_CS_PIN     GPIO_NUM_15
#define EPD_DC_PIN     GPIO_NUM_27
#define EPD_RST_PIN    GPIO_NUM_26
#define EPD_CLK_PIN    GPIO_NUM_13
#define EPD_PWR_PIN    GPIO_NUM_16
#define EPD_MOSI_PIN   GPIO_NUM_14
#define EPD_BUSY_PIN   GPIO_NUM_25

static const char* TAG = "epd";
static spi_device_handle_t spi = NULL;

static const unsigned char VOLTAGE_FRAME[] = {
	0x6, 0x3F, 0x3F, 0x11, 0x24, 0x7, 0x17,
};

static const unsigned char LUT_VCOM[] = {	
	0x0,	0xF,	0xF,	0x0,	0x0,	0x1,	
	0x0,	0xF,	0x1,	0xF,	0x1,	0x2,	
	0x0,	0xF,	0xF,	0x0,	0x0,	0x1,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
};						

static const unsigned char LUT_WW[] = {	
	0x10,	0xF,	0xF,	0x0,	0x0,	0x1,	
	0x84,	0xF,	0x1,	0xF,	0x1,	0x2,	
	0x20,	0xF,	0xF,	0x0,	0x0,	0x1,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
};

static const unsigned char LUT_BW[] = {	
	0x10,	0xF,	0xF,	0x0,	0x0,	0x1,	
	0x84,	0xF,	0x1,	0xF,	0x1,	0x2,	
	0x20,	0xF,	0xF,	0x0,	0x0,	0x1,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
};

static const unsigned char LUT_WB[] = {	
	0x80,	0xF,	0xF,	0x0,	0x0,	0x3,	
	0x84,	0xF,	0x1,	0xF,	0x1,	0x4,	
	0x40,	0xF,	0xF,	0x0,	0x0,	0x3,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
};

static const unsigned char LUT_BB[] = {	
	0x80,	0xF,	0xF,	0x0,	0x0,	0x1,	
	0x84,	0xF,	0x1,	0xF,	0x1,	0x2,	
	0x40,	0xF,	0xF,	0x0,	0x0,	0x1,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	
};

static void spi_pre_cb_handler(spi_transaction_t *t)
{
	int dc = (int)t->user;
	gpio_set_level(EPD_DC_PIN, dc);
}

static void send_cmd(unsigned char cmd)
{
	spi_transaction_t t;

	memset(&t, 0, sizeof(t));	
	t.length = 8;
	t.tx_data[0] = cmd;
	t.user = (void*)0; 
	t.flags = SPI_TRANS_USE_TXDATA;

	spi_device_polling_transmit(spi, &t);
}

static void send_data(unsigned char data)
{
	spi_transaction_t t;

	memset(&t, 0, sizeof(t));	
	t.length = 8;
	t.tx_data[0] = data;
	t.user = (void*)1; 
	t.flags = SPI_TRANS_USE_TXDATA;

	spi_device_polling_transmit(spi, &t);
}

static inline void wait_until_idle(void)
{
	int busy;

	ESP_LOGI(TAG, "display busy...");
	do {
		vTaskDelay((TickType_t) 20 / portTICK_PERIOD_MS);
		send_cmd(0x71);
		busy = gpio_get_level(EPD_BUSY_PIN);
	} while (busy == 0);

	vTaskDelay((TickType_t) 200 / portTICK_PERIOD_MS);
	ESP_LOGI(TAG, "display ready");
}

static inline void reset(void)
{
	gpio_set_level(EPD_RST_PIN, 1);
	vTaskDelay((TickType_t) 200 / portTICK_PERIOD_MS);

	gpio_set_level(EPD_RST_PIN, 0);
	vTaskDelay((TickType_t) 2 / portTICK_PERIOD_MS);

	gpio_set_level(EPD_RST_PIN, 1);
	vTaskDelay((TickType_t) 200 / portTICK_PERIOD_MS);
}

static inline void config_lut(unsigned char cmd, const unsigned char *lut)
{
	send_cmd(cmd);

	for (int i = 0; i < 42; i++)
		send_data(lut[i]);
}

void epd_init(void)
{
	gpio_config_t io_cfg = {
		.pin_bit_mask = ((1ULL << EPD_DC_PIN) |
		                (1ULL << EPD_PWR_PIN) |
		                (1ULL << EPD_RST_PIN) |
		                (1ULL << EPD_BUSY_PIN)),
		.pull_up_en = true
	};

	ESP_ERROR_CHECK(gpio_config(&io_cfg));

	ESP_ERROR_CHECK(gpio_set_direction(EPD_DC_PIN, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(EPD_PWR_PIN, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(EPD_RST_PIN, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(EPD_BUSY_PIN, GPIO_MODE_INPUT));

	gpio_set_level(EPD_PWR_PIN, 1);

	spi_bus_config_t bus_cfg = {
		.miso_io_num = -1,
		.mosi_io_num = EPD_MOSI_PIN,
		.sclk_io_num = EPD_CLK_PIN,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = EPD_WIDTH * EPD_HEIGHT
	};

	ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

	spi_device_interface_config_t dev_cfg = {
		.clock_speed_hz = 10 * 1000 * 1000,
		.mode = 0,
		.spics_io_num = EPD_CS_PIN,
		.queue_size = 3,
		.pre_cb = spi_pre_cb_handler
	};

	ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &spi));
}

void epd_wake(void)
{
	reset();

	send_cmd(0x01);
	send_data(0x17);

	send_data(*(VOLTAGE_FRAME + 6));
	send_data(*(VOLTAGE_FRAME + 1));
	send_data(*(VOLTAGE_FRAME + 2));
	send_data(*(VOLTAGE_FRAME + 3));


	send_cmd(0x82);
	send_data(*(VOLTAGE_FRAME + 4));

	send_cmd(0x06);
	send_data(0x27);
	send_data(0x27);
	send_data(0x2F);
	send_data(0x17);

	send_cmd(0x04);
	vTaskDelay((TickType_t) 100 / portTICK_PERIOD_MS);
	wait_until_idle();	

	send_cmd(0x00);
	send_data(0x3F);

	send_cmd(0x61);
	send_data(0x03);
	send_data(0x20);
	send_data(0x01);
	send_data(0xE0);

	send_cmd(0x15);
	send_data(0x00);

	send_cmd(0x50);
	send_data(0x10);
	send_data(0x00);

	send_cmd(0x60);
	send_data(0x22);

	send_cmd(0x65);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);

	config_lut(0x20, LUT_VCOM);
	config_lut(0x21, LUT_WW);
	config_lut(0x22, LUT_BW);
	config_lut(0x23, LUT_WB);
	config_lut(0x24, LUT_BB);
}

static inline void refresh()
{
	send_cmd(0x12);
	vTaskDelay((TickType_t) 100 / portTICK_PERIOD_MS);
	wait_until_idle();
}

void epd_clear(void)
{
	int height = EPD_HEIGHT;
	int width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8 ) : (EPD_WIDTH / 8 + 1);

	send_cmd(0x10);
	for(int i = 0; i < height * width; i++)
		send_data(0xFF);

	send_cmd(0x13);
	for(int i = 0; i < height * width; i++)
		send_data(0x00);

	refresh();
}

void epd_draw(const unsigned char *buf, int n)
{
	send_cmd(0x10);
	for (int i = 0; i < n; i++)
		send_data(~buf[i]);

	send_cmd(0x13);
	for (int i = 0; i < n; i++)
		send_data(buf[i]);

	refresh();
}

void epd_sleep(void)
{
	send_cmd(0x02);
	wait_until_idle();
	send_cmd(0x07);
	send_data(0xA5);
}


