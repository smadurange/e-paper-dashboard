#ifndef SCRN_H
#define SCRN_H

struct scrn {
	int width;
	int height;
	unsigned char *fb;
};

struct sprite {
	int width;
	int height;
	int offset_x;
	int offset_y;
	const unsigned char *bmp;
};

void scrn_clear(struct scrn *sc);

void scrn_draw(struct scrn *sc, struct sprite *s);

#endif /* SCRN_H */
