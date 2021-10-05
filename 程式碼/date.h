/* 各函數功能請參閱 date.c */

typedef struct date {
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char week;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
} DATE;

void long_to_date(unsigned long *time, DATE * date);
void show_date(char line, unsigned long *time, __bit type) __reentrant;
__idata extern volatile unsigned long current_time;
void date_to_long(DATE * date, unsigned long *time);
unsigned long adjust_time(unsigned long time);
