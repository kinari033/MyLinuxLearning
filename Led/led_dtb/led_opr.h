#ifndef _LED_OPR_H
#define _LED_OPR_H

#define GROUP(x)    (x>>16)
#define PIN(x)      (x&0xFFFF)
#define GROUP_PIN(g,p) ((g<<16) | (p))

extern int g_ledpins[100];

struct led_operation {
    int (*init)(int which);
    int (*ctl)(int which, char status);
};

void register_led_opr(struct led_operation *opr);


#endif //  _LED_OPR_H


