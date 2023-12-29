#ifndef GUI_H
#define GUI_H

#include "scrn.h"
#include "stock.h"

#define MAXLEN 48000

void gui_draw_temp(struct scrn *sc);

void gui_draw_humid(struct scrn *sc);

void gui_draw_layout(struct scrn *sc);

void gui_draw_date(struct scrn *sc, struct tm *now);

int gui_draw_str(struct scrn *sc, const char *s, int x0, int y0, int x_max, int y_max, int bold);

void gui_plot_stocks(struct scrn *sc, struct stock_item *data);

#endif /* GUI_H */
