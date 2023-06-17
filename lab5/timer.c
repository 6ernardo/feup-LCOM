#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int global_counter = 0, timer_hook_id = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  uint8_t st;
  if(timer_get_conf(timer, &st) != 0) return 1; //Guardar a config de maneira a preservar os 4 LSB

  uint8_t control_word = BIT(5) | BIT(4) | (st & 15); //counter initialization e manter os 4 lsb. Timer ainda não esta selecionado

  //Ativar bit na control word que equivale ao timer desejado e igualar timer_macro ao endereço do timer
  int timer_macro;
  if(timer == 0){
    //control_word = control_word;
    timer_macro = TIMER_0;
  }
  else if(timer == 1){
    control_word = control_word | BIT(6);
    timer_macro = TIMER_1;
  }
  else if(timer == 2){
    control_word = control_word | BIT(7);
    timer_macro = TIMER_2;
  }
  else return 1;

  if(sys_outb(TIMER_CTRL, control_word) != 0) return 1; //escrever word no i8254

  uint16_t counter = TIMER_FREQ / freq; //valor que devemos carregar em counter para obter a frequencia desejada
  uint8_t lsb, msb;
  if(util_get_LSB(counter, &lsb) != 0) return 1; //apenas se pode carregar o counter 8 bits de cada vez então o counter
  if(util_get_MSB(counter, &msb) != 0) return 1; //é divido em lsb e msb, e esses valores são escritos individualmente no timer

  
  if(sys_outb(timer_macro, lsb) != 0) return 1; //escrever lsb seguido de msb no timer pretendido
  if(sys_outb(timer_macro, msb) != 0) return 1;

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(timer_hook_id); // mask para hook id

  return sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer_hook_id); //subscreve interrupts
}

int (timer_unsubscribe_int)() {
  return sys_irqrmpolicy(&timer_hook_id); //cancela sub de interrupts
}

void (timer_int_handler)() {
  global_counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  uint8_t read_back_command = BIT(7) | BIT(6) | BIT(5) | BIT(timer+1); //bit 7,6 para Read-back cmd, bit 5 para desativar leitura da count
  if(sys_outb(TIMER_CTRL, read_back_command) != 0) return 1; //escreve na porta do i8254 que quero ler a config do timer
  if(util_sys_inb(TIMER_0 + timer, st) != 0) return 1; //usa wrapper para ler config to timer, guarda em st

  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  union timer_status_field_val val;
  
  switch (field)
  {
  case tsf_all:
    val.byte = st;

    break;

  case tsf_initial:
    st = (st >> 4) & 3; // st so tem bits 4 e 5

    if(st == 1) val.in_mode = LSB_only;
    else if(st == 2) val.in_mode = MSB_only;
    else if(st == 3) val.in_mode = MSB_after_LSB;
    else val.in_mode = INVAL_val;

    break;

  case tsf_mode:
    st = (st >> 1) & 7; // 7=111 -> Com esta operação st vai ter apenas os bits 1,2,3

    if(st == 6) val.count_mode = 2;
    else if(st == 7) val.count_mode = 3;
    else val.count_mode = st;

    break;

  case tsf_base:
    st = st & 1;

    val.bcd = st;

    break;
  
  default:
    return 1;
  }

  return timer_print_config(timer, field, val);
}
