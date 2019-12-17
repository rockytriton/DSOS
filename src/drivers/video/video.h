#pragma once

#include <common.h>

void video_init();
int font_get_height();
int font_get_width();
bool font_get_pixel(char ch, dword x, dword y);
