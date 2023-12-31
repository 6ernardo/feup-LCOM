#include <lcom/lcf.h>
#include "graphics.h"

xpm_image_t img;
// estrutura de dados que contém informação sobre o modo gráfico
vbe_mode_info_t mode_info; 

// resolução horizontal
uint8_t HRES;

//static unsigned BytesPerPixel = (mode_info.BitsPerPixel + 7) / 8;

int set_graphic_mode(uint16_t submode) {
    reg86_t reg86;
    memset(&reg86, 0, sizeof(reg86)); // inicialização da estrutura com o valor 0 em todos os parâmetros
    reg86.intno = 0x10;               // intno é sempre 0x10      
    reg86.ah = 0x4F;                  // parte mais significativa de AX
    reg86.al = 0x02;                  // parte menos significativa de AX. 0x02 no caso de modo gráfico
    // reg86.ax = 0x4F02;             // equivamente às duas últimas instruções
    reg86.bx = submode | BIT(14);     // determinação do submodo com memória linear
    if (sys_int86(&reg86) != 0) {     // se houver algum erro, abortar a função
        printf("Set graphic mode failed\n");
        return 1;
    }
    return 0;
}
int set_text_mode() {
    reg86_t reg86;                       
    memset(&reg86, 0, sizeof(reg86));   // inicialização da estrutura com o valor 0 em todos os parâmetros
    reg86.intno = 0x10;                 // intno é sempre 0x10 
    reg86.ah = 0x00;                    // parte mais significativa de AX 
    reg86.al = 0x03;                    // parte menos significativa de AX. 0x03 no caso de modo texto
    // reg86.ax = 0x0003;               // equivamente às duas últimas instruções
    reg86.bx = 0x0000;                  // não há submodo no modo de texto
    if(sys_int86(&reg86) != 0) {        // se houver algum erro, abortar a função
        printf("Set text mode failed\n");
        return 1;
    }
    return 0;
}
/* função de mapeamento da VRAM */
int set_frame_buffer(uint16_t mode, uint8_t **frame_buffer){

  // retirar informação sobre o @mode
  memset(&mode_info, 0, sizeof(mode_info));
  if(vbe_get_mode_info(mode, &mode_info)) return 1;

  // cálculo do tamanho do frame buffer, em bytes
  uint8_t bytes_per_pixel = (mode_info.BitsPerPixel + 7) / 8;
  unsigned int frame_size = mode_info.XResolution * mode_info.YResolution * bytes_per_pixel;
  
  // preenchimento dos endereços físicos
  struct minix_mem_range physic_addresses;
  physic_addresses.mr_base = mode_info.PhysBasePtr; // início físico do buffer
  physic_addresses.mr_limit = physic_addresses.mr_base + frame_size; // fim físico do buffer
  
  // alocação física da memória necessária para o frame buffer
  if (sys_privctl(SELF, SYS_PRIV_ADD_MEM, &physic_addresses)) {
    printf("Physical memory allocation error\n");
    return 1;
  }

  // alocação virtual da memória necessária para o frame buffer
  *frame_buffer = vm_map_phys(SELF, (void*) physic_addresses.mr_base, frame_size);
  if (*frame_buffer == NULL) {
    printf("Virtual memory allocation error");
    return 1;
  }

  return 0;
}
int paint_pixel(uint16_t x, uint16_t y, uint32_t color, uint8_t *frame_buffer) {
  if(color == 0xfffffe) return 0;
  // As coordenadas têm de ser válidas
  if(x >= mode_info.XResolution || y >= mode_info.YResolution) return 1;

  // Cálculo dos Bytes per pixel da cor escolhida. Arredondamento por excesso.
  unsigned BytesPerPixel = (mode_info.BitsPerPixel + 7) / 8;

  // Índice (em bytes) da zona do píxel a colorir
  unsigned int index = (mode_info.XResolution * y + x) * BytesPerPixel;

  // A partir da zona de memória frame_buffer[index], copia @BytesPerPixel bytes da @color
  if (memcpy(frame_buffer+index, &color, BytesPerPixel) == NULL) return 1;

  return 0;
}
int draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color, uint8_t *frame_buffer){
  if(color == 0xfffffe) return 0;
  for (int i=0; i < len; i++){
    if(paint_pixel(x+i,y,color, frame_buffer)!=0){
      return 1;
    }
  }
  return 0;
}
int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t *frame_buffer){
  if(color == 0xfffffe) return 0;
  for(int i = 0; i < height; i++){ 
    if(draw_hline(x,y+i,width, color, frame_buffer )!=0){
      vg_exit();
      return 1;}
  }  
  return 0;
}


int invert_pixel_color(uint16_t x, uint16_t y, uint8_t *frame_buffer) {
  // As coordenadas têm de ser válidas
 if (x >= mode_info.XResolution || y >= mode_info.YResolution) return 1;

  // Cálculo dos Bytes per pixel da cor escolhida (24 bits = 3 bytes)
  unsigned BytesPerPixel = 3;

  // Índice (em bytes) da zona do píxel a inverter
  unsigned int index = (mode_info.XResolution * y + x) * BytesPerPixel;

  // Ponteiro para a cor atual do pixel no frame_buffer
  uint8_t *pixel_color = frame_buffer + index;

  // Inverter a cor de 24 bits
  pixel_color[0] = ~pixel_color[0];
  pixel_color[1] = ~pixel_color[1];
  pixel_color[2] = ~pixel_color[2];

  return 0;
} 
int convert_to_gray(uint16_t x, uint16_t y, uint8_t *frame_buffer) {
  // As coordenadas têm de ser válidas
  if(x >= mode_info.XResolution || y >= mode_info.YResolution) return 1;

  // Cálculo dos Bytes per pixel da cor escolhida. Arredondamento por excesso.
  unsigned BytesPerPixel = (mode_info.BitsPerPixel + 7) / 8;

  // Índice (em bytes) da zona do píxel a converter para cinza
  unsigned int index = (mode_info.XResolution * y + x) * BytesPerPixel;

  // Ponteiro para a cor atual do pixel no frame_buffer
  uint32_t *pixel_color = (uint32_t*)(frame_buffer + index);

  // Extrair os componentes de cor (vermelho, verde, azul) do pixel
  uint8_t red = (*pixel_color >> 16) & 0xFF;
  uint8_t green = (*pixel_color >> 8) & 0xFF;
  uint8_t blue = *pixel_color & 0xFF;

  // Calcular o valor médio dos componentes de cor para obter o tom de cinza
  uint8_t gray = (red + green + blue) / 3;

  // Criar uma cor em tons de cinza com os mesmos valores para os componentes de cor
  uint32_t gray_color = (gray << 16) | (gray << 8) | gray;

  // Substituir a cor original no pixel pela cor em tons de cinza
  *pixel_color = gray_color;

  return 0;
}
