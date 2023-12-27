#include <esp_log.h>

#include "scrn.h"

#define MASK_ON(x) (1ULL << (7 - (x % 8)))
#define MASK_OFF(x) ~(1ULL << (7 - (x % 8)))

const static char *TAG = "scrn";

void scrn_clear(struct scrn *sc)
{
	int n = sc->width * sc->height / 8;

	for (int i = 0; i < n; i++)
		sc->fb[i] = 0;
}

void scrn_draw(struct scrn *sc, struct sprite *s)
{
	int s_n = s->width * s->height;
	int buf_n = sc->width * sc->height;
	int buf_i = s->offset_y * sc->width + s->offset_x;

	int s_i = 0, s_col = 0;

	while (s_i < s_n) {
		if (s_col == s->width) {
			s_col = 0;
			buf_i += sc->width - s->width;
		}

		if (s->bmp[s_i / 8] & MASK_ON(s_i)) {
			if (buf_i < buf_n)
				sc->fb[buf_i / 8] |= MASK_ON(buf_i);
			else
				ESP_LOGE(TAG, "draw(): pixel out of range");
		}

		s_i++;
		buf_i++;
		s_col++;
	}
}
