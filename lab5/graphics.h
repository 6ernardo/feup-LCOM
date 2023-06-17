#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <lcom/lcf.h>

int set_graph_mode(uint16_t mode);

int set_frame_buffer(uint16_t mode);

uint32_t normalize_color(uint32_t color);

int vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color);

int vg_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

int vg_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

uint32_t (direct_mode)(uint32_t R, uint32_t G, uint32_t B);
uint32_t (indexed_mode)(uint16_t col, uint16_t row, uint8_t step, uint32_t first, uint8_t n);
uint32_t (Red)(unsigned j, uint8_t step, uint32_t first);
uint32_t (Green)(unsigned i, uint8_t step, uint32_t first);
uint32_t (Blue)(unsigned j, unsigned i, uint8_t step, uint32_t first);
uint32_t (R)(uint32_t first);
uint32_t (G)(uint32_t first);
uint32_t (B)(uint32_t first);

int print_xpm(xpm_map_t xpm, uint16_t x, uint16_t y);

#endif /* _GRAPHICS_H_ */