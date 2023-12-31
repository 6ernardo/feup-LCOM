// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "mouse.h"
#include "i8254.h"
#include "kbc.h"

extern uint8_t byte;
extern struct packet pp;
extern int global_counter;

typedef enum {
  START,
  UP,
  VERTEX,
  DOWN,
  END
} State;

State state = START;

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {
    int ipc_status, r;
    uint8_t irq_set;
    message msg;

    if(mouse_subscribe_int(&irq_set) != 0) return 1; //subscribe mouse interrupts

    if(mouse_enable_data_reporting() != 0) return 1;

    while(cnt) { /* You may want to use a different condition */
        /* Get a request message. */
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                    if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
                        mouse_ih();
                        mouse_assemble_array();
                        if(byte == 3){ //leu os 3 bytes
                            mouse_assemble_packet();
                            mouse_print_packet(&pp);
                            byte = 0;
                            cnt--;
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

    if(mouse_write(0XF5) != 0) return 1;
    if(mouse_unsubscribe_int() != 0) return 1;

    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
    int ipc_status, r;
    uint8_t mouse_mask, timer_mask;
    message msg;
    uint8_t time;

    uint16_t frequency = sys_hz();

    if(mouse_subscribe_int(&mouse_mask) != 0) return 1; //subscribe mouse interrupts
    if(mouse_enable_data_reporting() != 0) return 1;

    if(timer_subscribe_int(&timer_mask) != 0) return 1; //subscribe timer

    while(time < idle_time) { /* You may want to use a different condition */
        /* Get a request message. */
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                    if (msg.m_notify.interrupts & mouse_mask) { //mouse interrupt
                        mouse_ih();
                        mouse_assemble_array();
                        if(byte == 3){ //leu os 3 bytes
                            mouse_assemble_packet();
                            mouse_print_packet(&pp);
                            byte = 0;
                        }
                        time = 0;
                        global_counter = 0;
                    }
                    if(msg.m_notify.interrupts & timer_mask){ //timer interrupt
                        timer_int_handler();
                        if(global_counter % frequency == 0) time++;
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

    if(mouse_write(0XF5) != 0) return 1;
    if(mouse_unsubscribe_int() != 0) return 1;

    if(timer_unsubscribe_int() != 0) return 1;

    return 0;
}

void (state_machine)(uint8_t tolerance){
    switch (state){
        case START:
            if(pp.lb && !pp.rb && !pp.mb){ //Apenas left button pressionado
                state=UP;
            }
            break;
        case UP:
        if()
    }
}

int (mouse_test_gesture)() {
    int ipc_status, r;
    uint8_t irq_set;
    message msg;

    if(mouse_subscribe_int(&irq_set) != 0) return 1; //subscribe mouse interrupts

    if(mouse_enable_data_reporting() != 0) return 1;

    while(state != END) { /* You may want to use a different condition */
        /* Get a request message. */
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                    if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
                        mouse_ih();
                        mouse_assemble_array();
                        if(byte == 3){ //leu os 3 bytes
                            mouse_assemble_packet();
                            mouse_print_packet(&pp);
                            byte = 0;
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

    if(mouse_write(0XF5) != 0) return 1;
    if(mouse_unsubscribe_int() != 0) return 1;

    return 0;
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    /* This year you need not implement this. */
    printf("%s(%u, %u): under construction\n", __func__, period, cnt);
    return 1;
}