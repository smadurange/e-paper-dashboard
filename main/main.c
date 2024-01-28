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
#include "news.h"
#include "scrn.h"
#include "wifi.h"
#include "stock.h"

const static char *TAG = "app";

void app_main(void)
{
	time_t t;
	char ts[20];
	struct tm now;
	struct scrn sc;
	struct news_item *news;
	struct stock_item *stock = NULL;

	int ntp_rc = 0;

	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	sc.width = EPD_WIDTH;
	sc.height = EPD_HEIGHT;
	sc.fb = heap_caps_malloc(sizeof(sc.fb[0]) * MAXLEN, MALLOC_CAP_DMA);

	wifi_connect();

	ntp_init();
	dht_init();
	news_init();
	stock_init();
	epd_init();

	for (;;) {
		if (!ntp_rc)
			ntp_rc = ntp_sync();

		t = time(NULL);
		now = *localtime(&t);
		strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &now);

		gui_draw_layout(&sc);
		gui_draw_temp(&sc);
		gui_draw_humid(&sc);
		gui_draw_date(&sc, &now);

		stock = stock_get_item();
		if (stock)
			gui_plot_stocks(&sc, stock);

		news = news_local_get();
		if (news)
			gui_draw_str(&sc, news->title, 335, 40, 785, 184, 0);

		news = news_world_get();
		if (news)
			gui_draw_str(&sc, news->title, 335, 214, 785, 340, 0);

		epd_wake();
		vTaskDelay(500 / portTICK_PERIOD_MS);	
		epd_draw(sc.fb, MAXLEN);
		vTaskDelay(1000 / portTICK_PERIOD_MS);	
		epd_sleep();

		ESP_LOGI(TAG, "last updated at %s", ts);
		vTaskDelay(20 * 60 * 1000 / portTICK_PERIOD_MS);	
	}
}
