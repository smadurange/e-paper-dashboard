#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <time.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs_flash.h>

#include "dht.h"
#include "epd.h"
#include "gui.h"
#include "ntp.h"
#include "rss.h"
#include "scrn.h"
#include "wifi.h"
#include "stock.h"

const static char *TAG = "app";

void app_main(void)
{
	int prev_day;
	time_t t;
	char date[20];
	struct tm now;
	struct scrn sc;
	struct rss_item *rss;
	struct stock_item *stock;

	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	sc.width = EPD_WIDTH;
	sc.height = EPD_HEIGHT;
	sc.fb = heap_caps_malloc(sizeof(sc.fb[0]) * MAXLEN, MALLOC_CAP_DMA);

	wifi_connect();

	ntp_init();
	ntp_sync();
	dht_init();
	rss_init();
	stock_init();
	epd_init();

	t = time(NULL);
	now = *localtime(&t);
	prev_day = now.tm_mday;

	for (;;) {
		t = time(NULL);
		now = *localtime(&t);

		if (prev_day != now.tm_mday) {
			rss_update();
			stock_update();
			prev_day = now.tm_mday;
			strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &now);
			ESP_LOGI(TAG, "updated rss and stock data at %s", date);
		}

		gui_draw_layout(&sc);
		gui_draw_temp(&sc);
		gui_draw_humid(&sc);
		gui_draw_date(&sc, &now);

		stock = stock_get_item();
		if (stock)
			gui_plot_stocks(&sc, stock);

		rss = rss_get_item();
		if (rss) {
			int y_offset = gui_draw_str(&sc, rss->title, 335, 40, 785, 340, 1);
			if (rss->description)
				gui_draw_str(&sc, rss->description, 335, y_offset + 72, 785, 340, 0);
	  	}

		epd_wake();
		vTaskDelay(500 / portTICK_PERIOD_MS);	
		epd_draw(sc.fb, MAXLEN);
		vTaskDelay(500 / portTICK_PERIOD_MS);	
		epd_sleep();
		vTaskDelay(15 * 60 * 1000 / portTICK_PERIOD_MS);	
	}
}
