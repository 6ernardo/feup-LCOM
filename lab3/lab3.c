#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "kbc.h"
#include "keyboard.h"

#define ESC 0x81

extern uint8_t scancode;
extern int sysinb_calls;
extern int global_counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
    int ipc_status, r;
    uint8_t irq_set;
    message msg;
    uint8_t byte[2];
    bool flag = false;

    if(keyboard_subscribe_int(&irq_set) != 0) return 1; //subscribe keyboard interrupts

    while(1) { /* You may want to use a different condition */
        /* Get a request message. */
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                    if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
                        kbc_ih();
                        if(scancode == 0xE0){ //scancode de 2 bytes
                            //kbd_print_scancode(!(scancode & BIT(7)), 2, &scancode);
                            byte[0]=scancode;
                            flag = true;
                        }
                        else if(flag){
                            byte[1] = scancode;
                            kbd_print_scancode(!(scancode & BIT(7)), 2, byte);
                            flag = false;
                        }
                        else{ //scancode de 1 byte
                            kbd_print_scancode(!(scancode & BIT(7)), 1, &scancode);
                        }

                        if(scancode == ESC){ //solta tecla ESC
                            if(keyboard_unsubscribe_int() != 0) return 1;
                            if(kbd_print_no_sysinb(sysinb_calls) != 0) return 1;

                            return 0;
                        } 
                    }
                break;
                    default:
                break; /*no other notifications expected: do nothing */
            }
        } else { /* received a standard message, not a notification */
            /**/
            /* no standard messages expected: do nothing */
        }
    }

    return 1;
}

int(kbd_test_poll)() {

    while(scancode != ESC){
        if(read_KBC_output(KBC_OUTBUFFER, &scancode) != 0) return 1;
        else{
            if(scancode == 0xE0){ //scancode de 2 bytes
                kbd_print_scancode(!(scancode & BIT(7)), 2, &scancode);
            }
            else{ //scancode de 1 byte
                kbd_print_scancode(!(scancode & BIT(7)), 1, &scancode);
            }
        }
    }

    return keyboard_restore();
}

int(kbd_test_timed_scan)(uint8_t n) {
    int ipc_status, r, seconds=0;
    uint8_t irq_set_TIMER, irq_set_KEYBOARD;
    message msg;

    if(keyboard_subscribe_int(&irq_set_KEYBOARD) != 0) return 1; //subscribe keyboard interrupts
    if(timer_subscribe_int(&irq_set_TIMER) != 0) return 1; //subscribe timer interrupts

    while(seconds < n) { /* You may want to use a different condition */
        /* Get a request message. */
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                    if (msg.m_notify.interrupts & irq_set_KEYBOARD) { /* subscribed interrupt - keyboard*/
                        kbc_ih();
                        seconds = 0;
                        global_counter = 0;

                        if(scancode == 0xE0){ //scancode de 2 bytes
                            kbd_print_scancode(!(scancode & BIT(7)), 2, &scancode);
                        }
                        else{ //scancode de 1 byte
                            kbd_print_scancode(!(scancode & BIT(7)), 1, &scancode);
                        }

                        if(scancode == ESC){ //solta tecla ESC
                            if(keyboard_unsubscribe_int() != 0) return 1;
                            if(timer_unsubscribe_int() != 0) return 1;
                            if(kbd_print_no_sysinb(sysinb_calls) != 0) return 1;

                            return 0;
                        } 
                    }

                    if (msg.m_notify.interrupts & irq_set_TIMER) { /* subscribed interrupt - timer */
                        timer_int_handler();
                        if(global_counter % 60 == 0) seconds++;
                    }

                    break;
                default:
                    break; /*no other notifications expected: do nothing */
            }
        } else { /* received a standard message, not a notification */
            /**/
            /* no standard messages expected: do nothing */
        }
    }

    keyboard_unsubscribe_int();
    timer_unsubscribe_int();
    kbd_print_no_sysinb(sysinb_calls);

    return 1;
}
