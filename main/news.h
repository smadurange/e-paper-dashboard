#ifndef EPD_NEWS_H
#define EPD_NEWS_H

struct news_item {
	char *title;
};

void news_init(void);

void news_update(void);

struct news_item * news_local_get(void);

struct news_item * news_world_get(void);

#endif
