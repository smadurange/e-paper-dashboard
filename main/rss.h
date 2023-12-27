#ifndef RSS_H
#define RSS_H

struct rss_item {
	char *title;
	char *description;
};

void rss_init(void);

void rss_update(void);

struct rss_item * rss_get_item(void);

#endif
