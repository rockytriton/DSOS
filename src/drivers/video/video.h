#pragma once

#include <common.h>

typedef struct {
    int x;
    int y;
} VPoint;

typedef struct {
    dword width;
    dword height;
} VSize;

typedef struct {
    VPoint pos;
    VSize size;
} VRect;

void video_init();
int font_get_height();
int font_get_width();
bool font_get_pixel(char ch, dword x, dword y);

void video_set_brush(dword color);
void video_draw_line(VPoint from, VPoint to);
void video_set_resolution(dword x, dword y, dword bpp);
void video_draw_string(char *p, dword x, dword y);

void video_pre_init();
