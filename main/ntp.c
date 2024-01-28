#include <freertos/FreeRTOS.h>

#include <time.h>
#include <esp_log.h>
#include <esp_netif_sntp.h>

#include "ntp.h"

const static char *TAG = "ntp";

void ntp_init(void)
{
	setenv("TZ", "CST-8", 1);
	tzset();

	ESP_LOGI(TAG, "initializing SNTP");
	esp_sntp_config_t sntp_conf = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_SNTP_TIME_SERVER);
	ESP_ERROR_CHECK(esp_netif_sntp_init(&sntp_conf));
}

int ntp_sync(void)
{
	int retry = 0;
	const int retry_count = 15;

	while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT) {
		ESP_LOGI(TAG, "waiting for system time to be set... (%d/%d)", retry, retry_count);
		if (++retry >= retry_count) {
			ESP_LOGE(TAG, "Failed to sync system time");
			return 0;
		}
	}

	ESP_LOGI(TAG, "system time set");
	return 1;
}
