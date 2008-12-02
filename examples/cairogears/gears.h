#ifndef _GEARS_H_
#define _GEARS_H_

struct gears;

struct gears *gears_create(float red, float green, float blue, float alpha);

void gears_draw(struct gears *gears, float angle);

#endif
