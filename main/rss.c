#include <freertos/FreeRTOS.h>

#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_tls.h>
#include <esp_http_client.h>
#include <sys/param.h>

#include "rss.h"

#define MAXLEN 12

extern const char rss_cert_pem_start[] asm("_binary_rss_cert_pem_start");
extern const char rss_cert_pem_end[]   asm("_binary_rss_cert_pem_end");

static const char* TAG = "rss";
static esp_http_client_handle_t http_client;

static int rss_i = 0;
static int rss_len = 0;
static struct rss_item *rss_items[MAXLEN];

static inline int is_blank(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n'; 
}

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

static inline void delete(void)
{
	for (int i = 0; i < rss_len; i++) {
		free(rss_items[i]->title);
		free(rss_items[i]->description);
		free(rss_items[i]);
	}

	rss_i = 0;
	rss_len = 0;
}

static inline void parse(char *xml)
{
	char *s;
	int i, j, rc;
	struct rss_item *item;
	
	s = xml;

	delete();
	
	// skip the main titles
	search(s, "<title>", "</title>", &i, &j);
	s += j + 1;
	search(s, "<title>", "</title>", &i, &j);
	s += j + 1;

	for (rss_len = 0; rss_len < MAXLEN; rss_len++) {
		rc = search(s, "<title>", "</title>", &i, &j);
		if (rc > 0) {
			item = malloc(sizeof(struct rss_item));
			if (!item) {
				ESP_LOGE(TAG, "malloc() failed for rss item");
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

			rc = search(s, "<description>", "</description>", &i, &j);
			if (rc < 0) {
				item->description = NULL;
			} else {
				item->description = malloc(sizeof(char) * (j - i + 2));
				if (!item->description) {
					ESP_LOGE(TAG, "malloc() failed for description");
					free(item->title);
					free(item);
					break;
				}

				copy(s, item->description, i, j);		
			}

			s += j + 1;
			rss_items[rss_len] = item;
		}
	}
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

struct rss_item * rss_get_item(void)
{
	struct rss_item *item = NULL;

	if (rss_i < rss_len) {
		item = rss_items[rss_i];
		rss_i = (rss_i + 1) % rss_len;
	}
	return item;
}

void rss_update(void)
{
	char *buf;
	esp_err_t rc;

	buf = NULL;
	esp_http_client_set_user_data(http_client, &buf);	

	for(;;) {
		rc = esp_http_client_perform(http_client);
		if (rc != ESP_ERR_HTTP_EAGAIN)
			break;
		vTaskDelay((TickType_t) 100 / portTICK_PERIOD_MS);
	}

	parse(buf);
	free(buf);
}

void rss_init(void)
{
	esp_http_client_config_t conf = {
		.url = "https://www.channelnewsasia.com/api/v1/rss-outbound-feed?_format=xml&category=10416",
		.is_async = true,
		.timeout_ms = 5000,
		.event_handler = http_evt_handler,
		.cert_pem = rss_cert_pem_start,
		.disable_auto_redirect = true,
	};

	http_client = esp_http_client_init(&conf);
	esp_http_client_set_header(http_client, "Accept", "application/rss+xml");

	rss_update();
}
