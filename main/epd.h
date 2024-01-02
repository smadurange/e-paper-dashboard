#ifndef EPD_H
#define EPD_H

#define EPD_WIDTH  800
#define EPD_HEIGHT 480

void epd_init(void);
void epd_clear(void);
void epd_draw(const unsigned char *buf, int n);
void epd_wake(void);
void epd_sleep(void);

#endif /* EPD_H */
