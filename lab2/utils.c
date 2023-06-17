#include <lcom/lcf.h>

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  if(lsb == NULL) return 1;

  *lsb = val & 255;

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  if(msb == NULL) return 1;

  *msb = (val >> 8) & 255;

  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  if(value == NULL) return 1;
  uint32_t val;
  int ret = sys_inb(port, &val); //sys_inb lê da porta, mete o valor lido em val
  *value = (uint8_t)val;

  return ret;
}
