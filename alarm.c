#include "input.h"
#include <8052.h>
#include "def.h"
#include "alarm.h"
#include "lcd.h"
#include "date.h"
#include <stdlib.h>

/* 是否於鈴響時，自動把鬧鐘設定從 On 改為 Off */
__idata volatile char auto_turn_off = -1;

/* 賴床倒數計時 */
volatile __bit snooze = 0;

/* 距下次鈴響的時間 */
__idata volatile long next = 0;

/* 鬧鐘設定的陣列 */
__idata volatile int Alarm[ALARM_CLOCK_NUMBER];
__idata volatile unsigned char Alarm_week[ALARM_CLOCK_NUMBER];

/* 目前是否於主畫面呼叫 calc_next() */
volatile __bit _in_main = 1;

/* 是否重新計算下次鈴響時間 */
volatile __bit recalc_alarm = 0;

/* 存放星期的字元 */
__code char week_ab[] = { 'S', 'M', 'T', 'W', 'T', 'F', 'S' };

/* 是否鈴響 */
volatile __bit ring = 0;

/* 解除鬧鐘。使用者可以回答問題，若回答正確，即可解除鬧鐘 
 */
__bit dismiss()
{
	unsigned int a;
	unsigned int b;
	unsigned int ans;

	/* 
	 * op: 加減法選擇
	 *  1: 減法
	 *  0: 加法
	 */
	__bit op;

	lcd_display_onoff(1, 1, 1);

	/* 隨機選擇加減法 */
	op = rand() < RAND_MAX / 2;

	/* 
	 * 隨機產生
	 * a + b <= M_MAX
	 * a >= M_MIN
	 * b >= M_MIN
	 */
	a = ((unsigned int) rand() + rand())
		% (M_MAX - 2 * M_MIN + 1) + M_MIN;

	b = ((unsigned int) rand() + rand())
		% (M_MAX - M_MIN - a + 1) + M_MIN;

	if (op) {
		/* 若為減法時，被減數則為 a + b */
		a = a + b;
	}

	/* 顯示題目 */
	lcd_clear_line(0);
	lcd_clear_line(1);
	lcd_cursor(0, 0);
	lcd_show_num(a, 5, 0, 0);
	lcd_putchar(op ? '-' : '+');
	lcd_show_num(b, 5, 0, 0);
	lcd_putchar('=');

	/* 等待使用者輸入 */
	ans = enter_number(5, 0, 0, 1, 0);
	lcd_display_onoff(1, 0, 0);

	if (op)
		return ans == a - b;
	else
		return ans == a + b;
}

/* 設定鬧鐘 */
void set_alarm_clock()
{
	/* 選擇之鬧鐘 */
	char select = 0;

	/* 程式狀態 */
	char state = 0;

	/* 
	 * 暫存用的變數
	 * selected_alarm 暫存 Alarm[] 。
	 * selected_alarm_week 暫存 Alarm_week[] 。
	 */
	int selected_alarm;
	char selected_alarm_week;
	char i;

	/* 存放自鍵盤輸入的資料 */
	char code;

	/* 存放輸入的數字 */
	unsigned int num;

	while (1) {
		selected_alarm = Alarm[select];
		selected_alarm_week = Alarm_week[select];

		/* == 顯示文字 begin == */
		lcd_cursor(0, 0);

		/* 顯示目前的鬧鈴編號 */
		lcd_putchar('(');
		lcd_show_num(select + 1, 2, 0, 1);
		lcd_putchar('/');
		lcd_show_num(ALARM_CLOCK_NUMBER, 2, 0, 1);
		lcd_putchar(')');

		/* 顯示時間 */
		lcd_puts("  Time: ");
		lcd_show_num(selected_alarm / 60, 2, 0, 1);
		lcd_putchar(':');
		lcd_show_num(selected_alarm % 60, 2, 1, 1);
		lcd_cursor(1, 0);

		/* 顯示星期 */
		lcd_puts("Day:  ");
		for (i = 0; i < 7; i++) {
			if ((selected_alarm_week & 0x01 << (7 - i)) != 0)
				lcd_putchar(week_ab[i]);
			else
				lcd_putchar(' ');
		}

		lcd_erase(0, 4, 0);

		/* 顯示 On/Off */
		if (selected_alarm_week & 0x01) {
			lcd_puts(" On");
		} else {
			lcd_puts("Off");
		}
		/* == 顯示文字 end == */

		/* 
		 * 0: 星期編輯
		 * 1: 編輯時間_時
		 * 2: 編輯時間_分
		 */
		switch (state) {
		case 0:
			/* 游標關閉 */
			lcd_display_onoff(1, 0, 0);
			break;
		case 1:
			/* 游標開啟，並停於「時」的位置 */
			lcd_display_onoff(1, 1, 1);
			lcd_cursor(0, 15);
			break;
		case 2:
			/* 游標開啟，並停於「分」的位置 */
			lcd_display_onoff(1, 1, 1);
			lcd_cursor(0, 18);
		}

		code = wait_for_code();
		/* 
		 * A: On/Off
		 * B: 返回主畫面
		 * D: 切換編輯項目 ( 星期 -> 時 -> 分 )
		 * E: 切換鬧鈴 ( 向前 )
		 * F: 切換鬧鈴 ( 向後 )
		 */
		switch (code) {
		case 0x0a:
			/* On/Off */
			selected_alarm_week ^= 0x01;
			break;
		case 0x0b:
			/* 返回主畫面 */
			lcd_display_onoff(1, 0, 0);
			return;
		case 0x0d:
			/* 切換編輯項目 */
			if (state == 2)
				state = 0;
			else
				state += 1;
			break;
		case 0x0e:
			/* 切換鬧鈴 ( 向前 ) */
			if (select >= ALARM_CLOCK_NUMBER - 1)
				select = 0;
			else
				select++;
			continue;
		case 0x0f:
			/* 切換鬧鈴 ( 向後 ) */
			if (select == 0)
				select = ALARM_CLOCK_NUMBER - 1;
			else
				select--;
			continue;
		default:
			/* 其他情形 ( 按下數字鍵 ) */
			if (state == 0) {
				/* 設定星期 */
				if (code < 0 || code > 6)
					continue;
				selected_alarm_week =
					selected_alarm_week ^ (0x01 <<
							       (7 - code));
			} else if (state == 1) {
				/* 設定時 */
				if (code < 0 || code > 9)
					continue;
				num = enter_number(2, 1, code, 1, 1);
				if (num > 23)
					continue;
				selected_alarm =
					num * 60 + (selected_alarm % 60);
				state = 2;
			} else {
				/* 設定分 */
				if (code < 0 || code > 9)
					continue;
				num = enter_number(2, 1, code, 1, 1);
				if (num > 59)
					continue;
				selected_alarm = selected_alarm / 60 * 60 + num;
				state = 0;
			}
		}
		/* 
		 * 把暫存用的變數 selected_alarm 、
		 * selected_alarm_week 回存
		 */
		Alarm[select] = selected_alarm;
		Alarm_week[select] = selected_alarm_week;
		recalc_alarm = 1;
	}
}

/* 計算下一個鈴響時間 */
void calc_next() __reentrant
{
	char i, k;
	long _next = 0;
	long next_tmp = -1;
	unsigned long cur_second;
	unsigned char _auto_turn_off = -1;

	/* 取得現在為一星期的哪一個時刻 */
	ET2 = 0;
	cur_second = current_time;
	ET2 = 1;
	cur_second =
		(unsigned
		 long) ((cur_second +
			 (unsigned long) (WEEK_OFFSET) * 86400) % 604800);
	// 86400*7=604800

	/* 使用迴圈檢查每一個鬧鈴設定 */
	for (i = 0; i < ALARM_CLOCK_NUMBER; i++) {
		/* 鬧鈴 On/Off 檢查 */
		if ((Alarm_week[i] & 0x01) == 0)
			continue;

		/* 若鬧鈴 On
		 * ，但沒有選擇星期，則在最接近的時刻鈴響一次 
		 */
		if (Alarm_week[i] == 0x01) {

			next_tmp = (long) Alarm[i] * 60 - cur_second % 86400;
			if (next_tmp <= 0)
				next_tmp += 86400;
			if (_next == 0 || next_tmp <= _next) {
				_next = next_tmp;
				_auto_turn_off = _in_main ? i : -1;
			}
		}

		/* 若鬧鈴 On ，有選擇星期 */
		for (k = 0; k < 7; k++) {
			if (((Alarm_week[i] >> 7 - k) & 0x01) == 0)
				continue;

			next_tmp =
				(long) 86400 *k + (long) Alarm[i] * 60 -
				cur_second;
			if (next_tmp <= 0)
				next_tmp += 604800;
			if (_next == 0 || next_tmp < _next) {
				_next = next_tmp;
				_auto_turn_off = -1;
			}
		}

	}

	/* 套用資料至全域變數 */
	ET2 = 0;

	/* 賴床模式 */
	if (snooze && (next < _next || _next == 0)) {
		_next = next;
		_auto_turn_off = -1;
	}

	/* 鈴響模式 */
	if (ring) {
		_auto_turn_off = -1;
		if (next == 0)
			next = MAX_RING;
		if (next >= _next && _next <= MAX_RING && _next != 0)
			_next = _next + MAX_RING;
		else
			_next = next;
	}

	next = _next;
	auto_turn_off = _auto_turn_off;
	ET2 = 1;
}
