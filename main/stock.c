#include <freertos/FreeRTOS.h>

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_tls.h>
#include <esp_http_client.h>
#include <sys/param.h>

#include "stock.h"

#define NAME_LEN 15
#define TIMESERIES_LEN 30
#define URL "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=%s&apikey=%s&datatype=csv"

extern const char stock_cert_pem_start[] asm("_binary_stock_cert_pem_start");
extern const char stock_cert_pem_end[]   asm("_binary_stock_cert_pem_end");

static char* TAG = "stock";
static esp_http_client_handle_t http_client;

static char *names[] = {
	CONFIG_STOCK_SYM_1,
	CONFIG_STOCK_SYM_2,
	CONFIG_STOCK_SYM_3,
	CONFIG_STOCK_SYM_4,
	CONFIG_STOCK_SYM_5,
	CONFIG_STOCK_SYM_6,
	CONFIG_STOCK_SYM_7,
	CONFIG_STOCK_SYM_8,
};

static int prices_ref[] = {
	CONFIG_STOCK_PRICE_1,
	CONFIG_STOCK_PRICE_2,
	CONFIG_STOCK_PRICE_3,
	CONFIG_STOCK_PRICE_4,
	CONFIG_STOCK_PRICE_5,
	CONFIG_STOCK_PRICE_6,
	CONFIG_STOCK_PRICE_7,
	CONFIG_STOCK_PRICE_8,
};

static const int names_len = sizeof(names) / sizeof(names[0]);

static int stocks_i = 0;
static int stocks_len = 0;
static struct stock_item **stocks = NULL;

static esp_err_t http_evt_handler(esp_http_client_event_t *evt)
{
	static int read_len;

	switch(evt->event_id) {
		case HTTP_EVENT_ON_HEADER:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s",
			              evt->header_key,
			              evt->header_value);
			break;
		case HTTP_EVENT_ON_DATA:
			int copy_len = 0;
			int cont_len = esp_http_client_get_content_length(evt->client);
			char **user_data = (char **) evt->user_data;

			if (*user_data == NULL) {
				*user_data = calloc(cont_len + 1, sizeof(char));
				if (!evt->user_data)
					ESP_LOGE(TAG, "calloc() failed for response buffer");
			}

			if (evt->user_data) {
				copy_len = MIN(evt->data_len, (cont_len - read_len));
				if (copy_len) {
					memcpy((*(char **) evt->user_data) + read_len, evt->data, copy_len);
					read_len += copy_len;
				}
				ESP_LOGD(TAG, "HTTP response size = %d bytes", read_len);
			}

			break;
		case HTTP_EVENT_ON_FINISH:
			read_len = 0;
			ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
			ESP_LOGD(TAG, "%s", *(char **) evt->user_data);
			ESP_LOGD(TAG, "response status=%d", esp_http_client_get_status_code(evt->client));
			break;
		case HTTP_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");

			int mbedtls_err = 0;
			esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t) evt->data,
			                                                 &mbedtls_err,
			                                                 NULL);
			if (err != 0) {
				ESP_LOGE(TAG, "Last esp error code: 0x%x", err);
				ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
			}
			read_len = 0;
			break;
		default:
			break;
	}

	return ESP_OK;
}

static inline void parse(char *s, char *name, int price)
{
	int i;
	int buflen = 10;
	char buf[buflen];

	int price_col = -1, col_n = 0, row_n = 0;

	int price_min = INT_MAX;
	int price_max = 0;

	char *tag = "close";
	
	for (i = 0; *s && *s != '\n' && price_col < 0; s++) {
		if (*s == ',') {
			col_n++;
			continue;
		}

		if (tag[i] && *s == tag[i]) {
			i++;
			if (!tag[i])
				price_col = col_n;
		} else
			i = 0;
	}

	if (price_col < 0) {
		ESP_LOGD(TAG, "string: \n%s", s);
		ESP_LOGE(TAG, "could not locate column %s", tag);
		return;
	}

	col_n = 0;

	while (*s && row_n < TIMESERIES_LEN) {
		if (*s == ',') {
			s++;
			col_n++;
			continue;
		}

		if (*s == '\n') {
			s++;
			col_n = 0;
			continue;
		}

		if (col_n != price_col) {
			s++;
			continue;
		}

		for (i = 0; *s && *s != ',' && *s != '\n' && i < buflen; i++, s++)
			buf[i] = *s;
		buf[i] = '\0';

		if (row_n == 0)
			snprintf(stocks[stocks_len]->name, NAME_LEN, "%s: %s", name, buf);

		int price = (int) (atof(buf) * 100);
		if (price < price_min)
			price_min = price;
		if (price > price_max)
			price_max = price;

		// csv arranges data in reverse chronological order
		stocks[stocks_len]->prices[TIMESERIES_LEN - 1 - row_n] = price;
		row_n++;
	}

	assert(row_n == TIMESERIES_LEN);

	stocks[stocks_len]->price_ref = price;
	stocks[stocks_len]->price_min = price_min;
	stocks[stocks_len]->price_max = price_max;

	stocks_len++;
}

void stock_update(int i)
{
	esp_err_t rc;

	char *buf = NULL;
	esp_http_client_set_user_data(http_client, &buf);	

	int urllen = snprintf(NULL, 0, URL, names[i], CONFIG_STOCK_API_KEY) + 1;

	char *url = malloc(sizeof(char) * urllen);
	if (!url) {
		ESP_LOGE(TAG, "malloc() failed for URL");
		return;
	}

	snprintf(url, urllen, URL, names[i], CONFIG_STOCK_API_KEY);
	esp_http_client_set_url(http_client, url);	

	for(;;) {
		rc = esp_http_client_perform(http_client);
		if (rc != ESP_ERR_HTTP_EAGAIN)
			break;
		vTaskDelay((TickType_t) 100 / portTICK_PERIOD_MS);
	}

	parse(buf, names[i], prices_ref[i]);

	free(url);
	free(buf);
}

struct stock_item * stock_get_item(void)
{
	struct stock_item *item = NULL;

	if (stocks_len < names_len) {
		stock_update(stocks_len);
	}

	if (stocks_i < stocks_len) {
		item = stocks[stocks_i];
		stocks_i = (stocks_i + 1) % names_len;
	}

	return item;
}

void stock_init(void)
{
	esp_http_client_config_t conf = {
		.url = URL,
		.is_async = true,
		.timeout_ms = 5000,
		.event_handler = http_evt_handler,
		.cert_pem = stock_cert_pem_start,
		.disable_auto_redirect = true,
	};

	http_client = esp_http_client_init(&conf);

	stocks = malloc(sizeof(struct stock_item *) * names_len);
	if (!stocks) {
		ESP_LOGE(TAG, "malloc() failed for stocks");
		return;
	}

	for (int i = 0; i < names_len; i++) {
		stocks[i] = malloc(sizeof(struct stock_item));
		if (!stocks[i]) {
			stocks[i] = NULL;
			ESP_LOGE(TAG, "malloc() failed for stock item");
			return;
		}

		stocks[i]->name = malloc(sizeof(char) * NAME_LEN);
		stocks[i]->prices = malloc(sizeof(int) * TIMESERIES_LEN);
		stocks[i]->prices_len = TIMESERIES_LEN;
	}
}
