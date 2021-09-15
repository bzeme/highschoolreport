/* 各函數功能請參閱 alarm.c */

extern __idata volatile char auto_turn_off;
extern volatile __bit snooze;
extern __idata volatile long next;
extern __idata volatile int Alarm[ALARM_CLOCK_NUMBER];
extern __idata volatile unsigned char Alarm_week[ALARM_CLOCK_NUMBER];
extern volatile __bit _in_main;
extern volatile __bit ring;
extern volatile __bit recalc_alarm;

void calc_next() __reentrant;
void set_alarm_clock();
__bit dismiss();
