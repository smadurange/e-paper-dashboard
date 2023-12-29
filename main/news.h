#ifndef NEWS_H
#define NEWS_H

struct news_item {
	char *title;
};

void news_init(void);

struct news_item * news_local_get(void);

struct news_item * news_world_get(void);

#endif /* NEWS_H */
