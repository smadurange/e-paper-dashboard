#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <esp_log.h>
#include <esp_tls.h>
#include <esp_http_client.h>
#include <sys/param.h>

#include "news.h"

#define MAXLEN 12
#define NEWS_FEED_UPDATED_BIT  BIT0
#define MUTEX_TIMEOUT ((TickType_t) 5000 / portTICK_PERIOD_MS)

extern const char news_cert_pem_start[] asm("_binary_news_cert_pem_start");
extern const char news_cert_pem_end[]   asm("_binary_news_cert_pem_end");

static const char* TAG = "news";

static QueueHandle_t news_evt_queue;
static SemaphoreHandle_t mutex;

static EventGroupHandle_t news_evt_group;

static esp_http_client_handle_t http_client;

static int news_local_i = 0;
static int news_local_len = 0;
static struct news_item *news_local[MAXLEN];

static int news_world_i = 0;
static int news_world_len = 0;
static struct news_item *news_world[MAXLEN];

static inline void copy(char *src, char *dst, int i, int j)
{
	int k;

	for (src += i, k = 0; i <= j; k++) {
		if (*src != '&') {
			dst[k] = *src++;
			i++;
		} else {
			src++;
			if (strncmp(src, "#034;", 5) == 0 || strncmp(src, "quot;", 5) == 0) {
				dst[k] = '"';
				src += 5;
				i += 6;
			} else if (strncmp(src, "#038;", 5) == 0) {
				dst[k] = '&';
				src += 5;
				i += 6;
			} else if (strncmp(src, "#039;", 5) == 0) {
				dst[k] = '\'';
				src += 5;
				i += 6;
			} else if (strncmp(src, "#060;", 5) == 0) {
				dst[k] = '<';
				src += 5;
				i += 6;
			} else if (strncmp(src, "#062;", 5) == 0) {
				dst[k] = '>';
				src += 5;
				i += 6;
			} else {
				ESP_LOGW(TAG, "Unknown escape sequence at %d\n", i);
				dst[k] = *src++;
				i++;
			}
		}
	}

	dst[k] = '\0';
}

static inline int search(char *s, char *from, char *to, int *a, int *b)
{
	char *tag;
	int i, j, is_a;

	i = 0;
	is_a = 1;
	tag = from;

start:
	j = 0;

	while (s[i] && s[i] != tag[j])
		i++;

	if (!is_a)
		*b = i - 1;

	while (s[i] && tag[j] && s[i] == tag[j]) {
		i++;
		j++;
	}

	if (s[i] && tag[j])
		goto start;

	if (is_a) {
		if (!s[i]) {
			*b = i;
			return -1;
		}

		*a = i;
		is_a = 0;
		tag = to;
		goto start;
	} else {
		if (!s[*b])
			return -1;
	
		// empty tag: advance to the end of the closing tag
		if (*b < *a) {
			*b += strlen(to) + 1;
			return -1;
		}

		return 1;
	}
}

static inline void delete(struct news_item **news, int *news_len, int *news_i)
{
	if (xSemaphoreTake(mutex, MUTEX_TIMEOUT) == pdTRUE) {
		for (int i = 0; i < *news_len; i++) {
			free(news[i]->title);
			free(news[i]);
		}

		*news_i = 0;
		*news_len = 0;

		xSemaphoreGive(mutex);
	} else
		ESP_LOGE(TAG, "delete() failed to acquire semaphore");
}

static inline void parse(char *xml, struct news_item **news, int *news_len, int *news_i)
{
	char *s;
	int i, j, rc;
	struct news_item *item;
	
	s = xml;

	delete(news, news_len, news_i);
	
	// skip the main titles
	search(s, "<title>", "</title>", &i, &j);
	s += j + 1;
	search(s, "<title>", "</title>", &i, &j);
	s += j + 1;

	if (xSemaphoreTake(mutex, MUTEX_TIMEOUT) == pdTRUE) {
		for (*news_len = 0; *news_len < MAXLEN; (*news_len)++) {
			rc = search(s, "<title>", "</title>", &i, &j);
			if (rc > 0) {
				item = malloc(sizeof(struct news_item));
				if (!item) {
					ESP_LOGE(TAG, "malloc() failed for news item");
					break;
				}

				item->title = malloc(sizeof(char) * (j - i + 2));
				if (!item->title) {
					ESP_LOGE(TAG, "malloc() failed for title");
					free(item);
					break;
				}
				
				copy(s, item->title, i, j);
				s += j + 1;
				news[*news_len] = item;
			}
		}
		xSemaphoreGive(mutex);
	} else
		ESP_LOGE(TAG, "parse() failed to acquire semaphore");
}

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

struct news_item * news_local_get(void)
{
	struct news_item *item = NULL;

	if (xSemaphoreTake(mutex, MUTEX_TIMEOUT) == pdTRUE) {
		if (news_local_i < news_local_len) {
			item = news_local[news_local_i];
			news_local_i = (news_local_i + 1) % news_local_len;
		}
		xSemaphoreGive(mutex);
	} else
		ESP_LOGE(TAG, "news_local_get() failed to acquire semaphore");

	return item;
}

struct news_item * news_world_get(void)
{
	struct news_item *item = NULL;

	if (xSemaphoreTake(mutex, MUTEX_TIMEOUT) == pdTRUE) {
		if (news_world_i < news_world_len) {
			item = news_world[news_world_i];
			news_world_i = (news_world_i + 1) % news_world_len;
		}
		xSemaphoreGive(mutex);
	} else
		ESP_LOGE(TAG, "news_world_get() failed to acquire semaphore");

	return item;
}

void news_update_task(void *arg)
{
	char *buf;
	time_t t;
	char ts[20];
	struct tm now;
	esp_err_t rc;

	for (;;) {
		buf = NULL;
		esp_http_client_set_user_data(http_client, &buf);

		// local news
		esp_http_client_set_url(http_client, "https://www.channelnewsasia.com/api/v1/rss-outbound-feed?_format=xml&category=10416");
		for(;;) {
			rc = esp_http_client_perform(http_client);
			if (rc != ESP_ERR_HTTP_EAGAIN)
				break;
			vTaskDelay((TickType_t) 100 / portTICK_PERIOD_MS);
		}

		parse(buf, news_local, &news_local_len, &news_local_i);
		free(buf);
		buf = NULL;

		// world news
		esp_http_client_set_url(http_client, "https://www.channelnewsasia.com/api/v1/rss-outbound-feed?_format=xml&category=6311");
		for(;;) {
			rc = esp_http_client_perform(http_client);
			if (rc != ESP_ERR_HTTP_EAGAIN)
				break;
			vTaskDelay((TickType_t) 100 / portTICK_PERIOD_MS);
		}

		parse(buf, news_world, &news_world_len, &news_world_i);
		free(buf);

		t = time(NULL);
		now = *localtime(&t);
		strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &now);
		ESP_LOGI(TAG, "updated news feed at %s", ts);
		xEventGroupSetBits(news_evt_group, NEWS_FEED_UPDATED_BIT);

		vTaskDelay(60 * 60 * 1000 / portTICK_PERIOD_MS);	
	}
}

void news_init(void)
{
	if((mutex = xSemaphoreCreateMutex()) == NULL) {
		ESP_LOGE(TAG, "xSemaphoreCreateMutex() failed");
		return;
	}

	news_evt_group = xEventGroupCreate();

	esp_http_client_config_t conf = {
		.url = "https://www.channelnewsasia.com",
		.is_async = true,
		.timeout_ms = 5000,
		.event_handler = http_evt_handler,
		.cert_pem = news_cert_pem_start,
		.disable_auto_redirect = true,
	};

	http_client = esp_http_client_init(&conf);
	esp_http_client_set_header(http_client, "Accept", "application/rss+xml");

	news_evt_queue = xQueueCreate(1, sizeof(int));
	xTaskCreate(news_update_task, "news_update_task", 4096, NULL, 10, NULL);	

	xEventGroupWaitBits(news_evt_group,
	                    NEWS_FEED_UPDATED_BIT,
	                    pdFALSE,
	                    pdFALSE,
	                    portMAX_DELAY);
}
