#include "graphics.h"

vbe_mode_info_t mode_info;
uint8_t *frame_buffer;

int set_graph_mode(uint16_t mode){
    reg86_t r86;
    
    /* Specify the appropriate register values */
    
    memset(&r86, 0, sizeof(r86));	/* zero the structure */

    r86.intno = 0x10; /* BIOS video services */
    r86.ah = 0x4F;    
    r86.al = 0x02;    
    r86.bx = mode | BIT(14);
    
    /* Make the BIOS call */

    if( sys_int86(&r86) != 0 ) {
        printf("\tvg_exit(): sys_int86() failed \n");
        return 1;
    }
    return 0;
}

int set_frame_buffer(uint16_t mode){
    
    memset(&mode_info, 0, sizeof(mode_info));

    if(vbe_get_mode_info(mode, &mode_info)) return 1;

    uint8_t bytes_per_pixel = (mode_info.BitsPerPixel + 7) / 8;
    unsigned int frame_size = mode_info.XResolution * mode_info.YResolution + bytes_per_pixel;

    struct minix_mem_range p_adr;
    p_adr.mr_base = mode_info.PhysBasePtr;
    p_adr.mr_limit = mode_info.PhysBasePtr + frame_size;

    if(sys_privctl(SELF, SYS_PRIV_ADD_MEM, &p_adr)) return 1;

    frame_buffer = vm_map_phys(SELF, (void*) p_adr.mr_base, frame_size);

    if(frame_buffer == NULL) return 1;

    return 0;
}

uint32_t normalize_color(uint32_t color){
    uint32_t new_color;

    if(mode_info.BitsPerPixel == 32) new_color = color;
    else new_color = color & (BIT(mode_info.BitsPerPixel) - 1);

    return new_color;
}

int vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color){
    unsigned BytesPerPixel = (mode_info.BitsPerPixel + 7) / 8;

    unsigned int index = (mode_info.XResolution * y + x) * BytesPerPixel;

    if(memcpy(&frame_buffer[index], &color, BytesPerPixel) == NULL) return 1;

    return 0;
}

int vg_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color){
    for(int i=0; i<len; i++){
        if(vg_draw_pixel(x+i, y, color) != 0) return 1;
    }

    return 0;
}

int vg_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color){
    for(int i=0; i<height; i++){
        if(vg_draw_hline(x, y+i, width, color) != 0){
            vg_exit();
            return 1;
        } 
    }

    return 0;
}

uint32_t (direct_mode)(uint32_t R, uint32_t G, uint32_t B) {
  return (R << mode_info.RedFieldPosition) | (G << mode_info.GreenFieldPosition) | (B << mode_info.BlueFieldPosition);
}

uint32_t (indexed_mode)(uint16_t col, uint16_t row, uint8_t step, uint32_t first, uint8_t n) {
  return (first + (row * n + col) * step) % (1 << mode_info.BitsPerPixel);
}

uint32_t (Red)(unsigned j, uint8_t step, uint32_t first) {
  return (R(first) + j * step) % (1 << mode_info.RedMaskSize);
}

uint32_t (Green)(unsigned i, uint8_t step, uint32_t first) {
  return (G(first) + i * step) % (1 << mode_info.GreenMaskSize);
}

uint32_t (Blue)(unsigned j, unsigned i, uint8_t step, uint32_t first) {
  return (B(first) + (i + j) * step) % (1 << mode_info.BlueMaskSize);
}

uint32_t (R)(uint32_t first){
  return ((1 << mode_info.RedMaskSize) - 1) & (first >> mode_info.RedFieldPosition);
}

uint32_t (G)(uint32_t first){
  return ((1 << mode_info.GreenMaskSize) - 1) & (first >> mode_info.GreenFieldPosition);
}

uint32_t (B)(uint32_t first){
  return ((1 << mode_info.BlueMaskSize) - 1) & (first >> mode_info.BlueFieldPosition);
}

int print_xpm(xpm_map_t xpm, uint16_t x, uint16_t y){
    xpm_image_t image;

    uint8_t *colors = xpm_load(xpm, XPM_INDEXED, &image);

    for(int i=0; i<image.height; i++){
        for(int j=0; j<image.width; j++){
            if(vg_draw_pixel(x+i, y+j, *colors) != 0) return 1;
            colors++;
        }
    }

    return 0;
}