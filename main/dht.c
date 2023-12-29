#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>

#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>

#include "dht.h"

#define DHT_DATA_LEN     40
#define DHT_DATA_BIT   BIT0
#define DHT_PIN GPIO_NUM_19

#define MUTEX_TIMEOUT ((TickType_t) 5000 / portTICK_PERIOD_MS)

static const char *TAG = "dht";

static int temp, humid;

static EventGroupHandle_t dht_evt_group;

static QueueHandle_t dht_evt_queue;
static SemaphoreHandle_t mutex;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

static inline int dht_await_pin_state(int state, int timeout)
{
	int t;
	static const uint16_t delta = 1;

	for (t = 0; t < timeout; t += delta) {
		ets_delay_us(delta);
		if (gpio_get_level(DHT_PIN) == state)
			return t;
	}
	return 0;
}

static inline int dht_get_raw_data(unsigned char buf[DHT_DATA_LEN])
{
	int rc;
	unsigned char i, pwl, pwh;

	gpio_set_level(DHT_PIN, 0);
	ets_delay_us(1100);
	gpio_set_level(DHT_PIN, 1);

	if (!dht_await_pin_state(0, 40)) {
		rc = 1;
		xQueueSend(dht_evt_queue, &rc, (TickType_t) 0);
		return 0;
	}

	if (!dht_await_pin_state(1, 80)) {
		rc = 2;
		xQueueSend(dht_evt_queue, &rc, (TickType_t) 0);
		return 0;
	}

	if (!dht_await_pin_state(0, 80)) {
		rc = 3;
		xQueueSend(dht_evt_queue, &rc, (TickType_t) 0);
		return 0;
	}

	for (i = 0; i < DHT_DATA_LEN; i++) {
		if (!(pwl = dht_await_pin_state(1, 50))) {
			rc = 4;
			xQueueSend(dht_evt_queue, &rc, (TickType_t) 0);
			return 0;
		}

		if (!(pwh = dht_await_pin_state(0, 70))) {
			rc = 5;
			xQueueSend(dht_evt_queue, &rc, (TickType_t) 0);
			return 0;
		}

		buf[i] = pwh > pwl;
	}

	return 1;
}

static inline int dht_verify_checksum(const unsigned char data[5])
{
	return data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF);
}

static inline int16_t dht_decode_data(uint8_t msb, uint8_t lsb)
{
	int16_t x;

	x = msb & 0x7F; 
	x <<= 8;
	x |= lsb;

	if (msb >> 7)
		x = 0 - x;

	return x;
} 

static inline int dht_get_data(unsigned char result[5])
{
	unsigned char i, rc, buf[DHT_DATA_LEN];
	
	taskENTER_CRITICAL(&spinlock);
	rc = dht_get_raw_data(buf);
	taskEXIT_CRITICAL(&spinlock);
	
	if (rc) {
		for (i = 0; i < DHT_DATA_LEN; i++) {
			result[i / 8] <<= 1;
			result[i / 8] |= buf[i];
		}

		if (dht_verify_checksum(result))
			return 1;
		else
			ESP_LOGI(TAG, "checksum failed");
	}

	return 0;
}

static void dht_err_task(void* arg)
{
	int rc;

	for (;;) {
		if (xQueueReceive(dht_evt_queue, &rc, portMAX_DELAY)) {
			switch (rc) {
			case 1:
				ESP_LOGE(TAG, "start sequence phase 1 error");
				break;
			case 2:
				ESP_LOGE(TAG, "start sequence phase 2 error");
				break;
			case 3:
				ESP_LOGE(TAG, "start sequence phase 3 error");
				break;
			case 4:
				ESP_LOGE(TAG, "low bit timeout");
				break;
			case 5:
				ESP_LOGE(TAG, "low bit timeout");
				break;
			default:
				ESP_LOGE(TAG, "unknown code %d", rc);
				break;
			}
		}
	}
}

void dht_read_task(void *arg)
{
	char data[5];
	uint16_t rh, tc;
	
	for (;;) {
		if (dht_get_data((unsigned char *) data)) {
			rh = dht_decode_data(data[0], data[1]);
			tc = dht_decode_data(data[2], data[3]);

			ESP_LOGD(TAG, "temperature: %dC, humidity: %d%%", tc, rh);

			if (xSemaphoreTake(mutex, MUTEX_TIMEOUT) == pdTRUE) {
				temp = tc;
				humid = rh;
				xSemaphoreGive(mutex);
			} else {
				ESP_LOGE(TAG, "dht_read_task(): failed to acquire semaphore");
			}
		}

		xEventGroupSetBits(dht_evt_group, DHT_DATA_BIT);
		vTaskDelay(5 * 60 * 1000 / portTICK_PERIOD_MS);	
	}
}

void dht_init(void)
{
	if((mutex = xSemaphoreCreateMutex()) == NULL) {
		ESP_LOGE(TAG, "xSemaphoreCreateMutex() failed");
		return;
	}

	dht_evt_group = xEventGroupCreate();

	gpio_config_t io_conf = {};	
	io_conf.pin_bit_mask = 1ULL << DHT_PIN;
	io_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;
	ESP_ERROR_CHECK(gpio_config(&io_conf));

	gpio_set_level(DHT_PIN, 1);
	vTaskDelay(1000 / portTICK_PERIOD_MS);

	dht_evt_queue = xQueueCreate(1, sizeof(int));
	xTaskCreate(dht_err_task, "dht_err_task", 4096, NULL, 10, NULL);	
	xTaskCreate(dht_read_task, "dht_read_task", 4096, NULL, 10, NULL);	

	xEventGroupWaitBits(dht_evt_group,
	                    DHT_DATA_BIT,
	                    pdFALSE,
	                    pdFALSE,
	                    portMAX_DELAY);
}

static inline void dht_ntos(int n, char s[5])
{
	char buf[4];

	snprintf(buf, sizeof buf, "%03d", n);

	s[0] = buf[0];
	s[1] = buf[1];
	s[2] = '.';
	s[3] = buf[2];
}

void dht_temp_get(char s[7])
{
	int tc = 0;

	if (xSemaphoreTake(mutex, MUTEX_TIMEOUT) == pdTRUE) {
		tc = temp;
		xSemaphoreGive(mutex);
	} else {
		ESP_LOGE(TAG, "dht_temp_get(): failed to acquire semaphore");
	}

	dht_ntos(tc, s);

	s[4] = 248;
	s[5] = 'C';
	s[6] = '\0';
}

void dht_humid_get(char s[6])
{
	int rh = 0;

	if (xSemaphoreTake(mutex, MUTEX_TIMEOUT) == pdTRUE) {
		rh = humid;
		xSemaphoreGive(mutex);
	} else {
		ESP_LOGE(TAG, "dht_temp_get(): failed to acquire semaphore");
	}

	dht_ntos(rh, s);

	s[4] = '%';
	s[5] = '\0';
}
