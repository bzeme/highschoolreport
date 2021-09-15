#include <8052.h>
#include "date.h"
#include "lcd.h"
#include "def.h"
#include "common.h"
#include "input.h"

/* 星期之文字 */
__code char *weeks[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

/* 月份之文字 */
__code char *month[] =
	{ "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT",
	"NOV", "DEC"
};

/* 每月天數 */
__code char month_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* 累積天數 */
__code int month_days2[] =
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

/* 現在時間 */
__idata volatile unsigned long current_time = 0;

void show_date(char line, unsigned long *time, __bit type) __reentrant
{
	DATE date;

	long_to_date(time, &date);
	lcd_cursor(line, 0);

	/* 選擇要顯示的格式 */
	if (type) {
		/* YYYY-MM-DD WWW HH:MM */
		lcd_show_num(1972 + date.year, 4, 1, 1);
		lcd_putchar('-');
		lcd_show_num(date.month + 1, 2, 1, 1);
		lcd_putchar('-');
		lcd_show_num(date.day, 2, 1, 1);
		lcd_putchar(' ');
		lcd_puts(weeks[date.week]);
		lcd_putchar(' ');
		lcd_show_num(date.hour, 2, 0, 1);
		lcd_putchar(':');
		lcd_show_num(date.minute, 2, 1, 1);
	} else {
		/* MMM DD WWW HH:MM:SS */
		lcd_puts(month[date.month]);
		lcd_putchar(' ');
		lcd_show_num(date.day, 2, 0, 1);
		lcd_putchar(' ');
		lcd_puts(weeks[date.week]);
		lcd_putchar(' ');
		lcd_putchar(' ');
		lcd_show_num(date.hour, 2, 0, 1);
		lcd_putchar(':');
		lcd_show_num(date.minute, 2, 1, 1);
		lcd_putchar(':');
		lcd_show_num(date.second, 2, 1, 1);
		lcd_putchar(' ');
	}
}

/* 把日期結構轉換成 32bit 長整數 */
void date_to_long(DATE * date, unsigned long *time)
{
	unsigned long _time = 0;

	/* 加上年份 */
	_time += ((date->year) / 4) * 86400 * (365 * 4 + 1);
	_time += ((date->year) % 100 % 4) * 86400 * 365;

	/* 加上日期 */
	_time += (month_days2[date->month] + date->day -
		  ((date->year % 4 == 0) && date->month <= 1)) * 86400;

	/* 加上時間 */
	_time += (unsigned long) 3600 *date->hour;

	_time += 60 * date->minute;
	_time += date->second;
	*time = _time;
}

/* 把 32bit 長整數轉換成日期結構 */
void long_to_date(unsigned long *time, DATE * out)
{
	/* 暫存 *time 變數 */
	unsigned long temp = *time;

	/* 存放日數 */
	unsigned int day = 0;

	/* 存放年份 */
	unsigned char year = 0;

	/* 存放該年是否為閏年 */
	unsigned char leap = 0;

	char i = 0;

	out->year = 0;

	/* 取得秒 */
	out->second = temp % 60;
	temp /= 60;

	/* 取得分 */
	out->minute = temp % 60;
	temp /= 60;

	/* 取得時 */
	out->hour = temp % 24;
	temp /= 24;

	/* 取得星期 */
	out->week = (temp + WEEK_OFFSET) % 7;

	/* 計算年份 */
	while ((day += (365 + (leap = (year % 4 == 0)))) <= temp)
		year++;
	out->year = year;

	/* 取得剩下的天數 */
	day = temp - (day - (365 + leap));

	for (i = 0; i < 12; i++) {
		if (day >= (month_days[i] + (i == 1 && leap))) {
			/* 若超過該月天數，則有下一個月 */
			day -= month_days[i] + (i == 1 && leap);
		} else {
			/* 若沒有超過該月天數，則i為月份 */
			out->month = i;
			out->day = day + 1;	/* 因日為從0開始計算，所以要加 
						 * 1 */
			return;
		}
	}
}

unsigned long adjust_time(unsigned long time)
{
	/* 
	 * 選取要編輯的項目
	 * 0 - 年
	 * 1 - 月
	 * 2 - 日
	 * 3 - 時
	 * 4 - 分
	 */

	/* 存放選擇要調整時間的項目 */
	char select = 0;

	/* 暫存輸入的時間 */
	unsigned int num;

	/* 以 DATE 格式存放時間 */
	DATE date;

	/* 存放該月的最大天數 */
	char day_max = 31;

	/* 暫存按鍵碼 */
	char code;

	/* 顯示游標 */
	lcd_display_onoff(1, 1, 1);

	long_to_date(&time, &date);
	date.second = 30;
	lcd_set_line(0, "Adjust Date and Time");
	while (1) {
		/* 進行日期格式轉換 long <-> DATE */
		date_to_long(&date, &time);
		long_to_date(&time, &date);
		show_date(1, &time, 1);

		switch (select) {
			/* 顯示對應游標位置 */
		case 0:
			lcd_cursor(1, 0);
			break;
		case 1:
			lcd_cursor(1, 5);
			break;
		case 2:
			lcd_cursor(1, 8);
			break;
		case 3:
			lcd_cursor(1, 15);
			break;
		case 4:
			lcd_cursor(1, 18);
			break;
		}

		code = wait_for_code();
		switch (code) {
		case 0x0d:
			/* 轉換游標位置 */
			if (select == 4)
				select = 0;
			else
				select += 1;
			break;
		case 0x0b:
			/* 不調整時間，取消 */
			return 0;
		case 0x0a:
			/* 調整完成 */
			return time;
		default:
			if (code < 10) {
				/* 若輸入為數字 */

				/* 若選擇調整年時，可輸入 4 碼，
				 * 其餘則輸入 2 碼 */
				num = enter_number(2 * ((select == 0) + 1), 1,
						   code, 1, 1);

				switch (select) {
					/* 依 select
					 * 變數之內容決定要調整的項目 
					 */
				case 0:
					/* 調整年 */
					if (num < 1972 || num > 2099)
						break;
					date.year = num - 1972;
					select++;
					break;
				case 1:
					/* 調整月 */
					if (num < 1 || num > 12)
						break;
					date.month = num - 1;
					select++;
					break;
				case 2:
					/* 調整日 */
					day_max =
						month_days[date.month] +
						(date.year % 4 == 0 &&
						 date.month == 1);
					if (num < 1 || num > day_max)
						break;
					date.day = num;
					select++;
					break;
				case 3:
					/* 調整時 */
					if (num > 23)
						break;
					date.hour = num;
					select++;
					break;
				case 4:
					/* 調整分 */
					if (num > 59)
						break;
					date.minute = num;
					select = 0;
				}
			}
		}
	}
}
