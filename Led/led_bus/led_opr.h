#ifndef _LED_OPR_H
#define _LED_OPR_H

struct led_operation
{
    int (*init)(int which);
    int (*ctl)(int which, char status);
};

struct led_operation *get_board_led_opr(void);


#endif // _LED_OPR_H