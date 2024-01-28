#ifndef STOCK_H
#define STOCK_H

struct stock_item {
	char *name;
	int *prices;
	int price_min;
	int price_max;
	int price_ref;
	int prices_len;
};

void stock_init(void);

struct stock_item *stock_get_item(void);

#endif /* STOCK_H */
