#include <8052.h>
#include "def.h"
#include "lcd.h"
#include "common.h"
#include "input.h"
#include "date.h"
#include <stdlib.h>
#include "alarm.h"

void timer2() __interrupt(5);

/* 百分之一秒計數 */
volatile unsigned char TimeCount = 0;

/* 閏百分之一秒計數 */
__idata volatile unsigned long leapCount = 0;

/* 是否計數 */
volatile __bit count = 0;

/* 可以重新更新時間 */
volatile __bit refresh = 0;

/* 設定日期格式 */
volatile char dateType = 0;

/* 標題 */
volatile char title = 0;

/* 存放標題字串 */
__code char *title_str[] = { "Clock", "Snooze", "Ringing" };

/* Timer初始化副程式 */
void initTimer()
{
	EA = 1;
	RCAP2H = (65536 - CLOCK_D12) / 256;
	RCAP2L = (65536 - CLOCK_D12) % 256;
	TH2 = RCAP2H;
	TL2 = RCAP2L;
	ET2 = 1;
	T2CON = 0x04;

	/* 等一下會用來產生亂數種子，所以也啟動 Timer 0 */
	TMOD = 0x01;
	TR0 = 1;
}

/* 中斷副程式 */
void timer2() __interrupt(5)
{
	TF2 = 0;
#if CLOCK_LEAP != 0
	/* 為了調整使時間誤差減少，加上了閏百分之一秒之計算 
	 */
	leapCount += CLOCK_LEAP;
	if (leapCount >= CLOCK_LEAP_MAX) {
		leapCount -= CLOCK_LEAP_MAX;
		return;
	}
#endif

	/* 計數 */
	if (count) {
		title = snooze;

		/* 鈴響判斷 */
		if (ring) {
			title = 2;
			BUZZ = !(TimeCount / 7 % 2 && TimeCount < 56);
		} else
			BUZZ = 1;

		TimeCount++;

		/* 若計數至秒 */
		if (TimeCount >= 100) {

			/* 倒數到下次鈴響時間 */
			if (next > 0) {
				next--;
				if (next == 0) {
					ring = !ring;
					snooze = 0;
					recalc_alarm = 1;
					if (auto_turn_off != -1) {
						Alarm_week[auto_turn_off] = 0;
						auto_turn_off = -1;
					}
				}
			}

			/* 在現在時間加上一秒鐘 */
			current_time++;

			/* 百分之一秒歸零 */
			TimeCount = 0;

			/* 可以重新更新時間 */
			refresh = 1;
		}
	} else
		BUZZ = 1;

}

/* 主桯式 */
int main(void)
{
	/* keydown
	 * 用來記錄鍵盤狀態，必須要在鍵盤沒有被按下，
	 * 之後才被按下的情形下才會動作 */
	__idata static char keydown = 1;

	/* 暫存鍵盤按下之變數 */
	__idata static unsigned char keycode;

	/* 
	 * 用來存放重設時間時之時間
	 * 由於 current_time 變數與中斷副程式共用，因此必須要在 ET2=0 時，
	 * 才能變更其值，再設定 ET2=1 ，以避免中斷發生在程式執行時，
	 * 產生不可遇期的結果
	 */
	__idata static unsigned long t = 0;

	/* 
	 * 用來存放現在時間 
	 * 由於 current_time 變數與中斷副程式共用，因此必須要在 ET2=0 時，
	 * 才能取得其值，再設定 ET2=1 ，以避免中斷發生在程式執行時，
	 * 產生不可遇期的結果
	 */
	__idata static unsigned long time = 0;

	/* 用來存放標題是否需要重新整理 */
	__bit trefresh = 1;

	/* 
	 * 暫存 refresh 、 title 變數
	 * 由於這些變數與中斷副程式共用，因此必須要在 ET2=0 時，
	 * 將其取出，再設定 ET2=1 ，以避免中斷發生在程式執行時，
	 * 產生不可遇期的結果
	 */
	__bit _refresh;
	__idata static char _title;

	/* 暫存 snoozing 計算之結果 */
	__idata static unsigned long tmp;

	/* 存放原來的標題，以利比較標題是否有更新 */
	__idata static char orig_title = 0;

	/* LCD 初始化與 Timer 初始化 */
	lcd_init();
	initTimer();

	/* 顯示歡迎訊息，並等候使用者按下任意鍵 */
	lcd_puts("Welcome!");
	wait_for_code();

	/* 由 Timer 0 之計數狀況產生亂數種子 */
	srand(TL0 * 256 + TH0);

	/* 已取得亂數種子， Timer 0 可以關閉了 */
	TR0 = 0;

	refresh = 1;
	/* 開始計數 */
	count = 1;
	lcd_display_onoff(1, 0, 0);

	while (1) {
		/* 進入省電模式，會在下次計時中斷時，喚醒
		 * CPU */
		PCON |= 1;

		/* 重新計算鬧鐘時間 */
		if (recalc_alarm) {
			calc_next();
			recalc_alarm = 0;
		}

		/* 
		 * 由於下面的程式會多次存取 recalc_alarm 、 refresh 變數，
		 * 為避免在中間發生中斷改變這兩個變數的值
		 * 所以先把它們存到區域變數
		 */
		_refresh = refresh;
		_title = title;

		/* 若標題不同，或是要求要重新整理標題時使用 
		 */
		if (orig_title != _title || trefresh) {
			lcd_set_line(0, title_str[_title]);
			orig_title = _title;
			trefresh = 0;
			_refresh = 1;
		}

		if (_refresh) {
			/* 
			 * 取得時間相關數據
			 *
			 * 由於這些變數與時間中斷副程式作用
			 * 加上這些資料型態需要用到 1 個指令以上去存取
			 * 所以要先關閉中斷再存取，避免存取這些資料時發生中斷，
			 * 才不會發生資料錯誤或其他不可遇期的錯誤
			 *
			 * 下面其他的 ET2=0 及 ET2=1 寫法也是一樣的作用
			 */
			ET2 = 0;
			time = current_time;
			tmp = next;
			ET2 = 1;

			lcd_cursor(0, 9);
			/* 重新整理 LCD 上的時間 */
			if (tmp > 0) {
				if (tmp > 86399) {
					lcd_show_num(tmp / 86400, 1, 0, 1);
					lcd_puts("d ");
				} else
					lcd_erase(0, 3, 0);

				tmp = tmp % 86400;
				lcd_show_num(tmp / 3600, 2, 0, 1);
				tmp = tmp % 3600;
				lcd_putchar(':');
				lcd_show_num(tmp / 60, 2, 1, 1);
				lcd_putchar(':');
				lcd_show_num(tmp % 60, 2, 1, 1);
			}

			refresh = 0;
			show_date(1, &time, dateType);
		}

		/* 取得按鍵狀態 */
		keycode = get_keycode();

		/* 表示放開按鍵，因此設 keydown=0 */
		if (keycode == 0xff) {
			keydown = 0;
			continue;
		}

		/* 若按鍵未放開，則不執行 */
		if (keydown)
			continue;

		/* 
		 * 依照按下的按鍵執行命令
		 * A: 切換日期時間格式
		 * B: 重新初始化LCD
		 * C: 鬧鐘設定
		 * D: 解除鬧鐘
		 * E: 進入賴床模式
		 */
		switch (keycode) {
		case 0x0b:
			/* 
			 * 若電路接於麵包板上，可能因為單芯線鬆脫使 LCD 被 RESET ，
			 * 此按鍵可以重新初始化 LCD 
			 */
			lcd_init();
			trefresh = 1;
			break;

		case 0x0c:
			/* 鬧鐘設定 */
			_in_main = 0;
			recalc_alarm = 1;
			set_alarm_clock();
			recalc_alarm = 1;
			trefresh = 1;
			_in_main = 1;
			break;

		case 0x0d:
			/* 解除鬧鐘 */
			if (!snooze && !ring)
				break;
			ET2 = 0;
			if (ring) {
				next = 60;
				snooze = 1;
				ring = 0;
				recalc_alarm = 1;
			}
			ET2 = 1;
			if (dismiss()) {
				ET2 = 0;
				snooze = 0;
				ring = 0;
				recalc_alarm = 1;
				ET2 = 1;
			}

			trefresh = 1;
			break;

		case 0x0e:
			/* 賴床模式 */
			if (!ring)
				break;
			ET2 = 0;
			ring = 0;
			next = MAX_SNOOZING;
			snooze = 1;
			ET2 = 1;
			recalc_alarm = 1;
			break;

		case 0x0f:
			/* 重設時間 */
			keydown = 1;
			if ((t = adjust_time(time)) != 0) {
				ET2 = 0;
				count = 0;
				TimeCount = 0;
				current_time = t;
				count = 1;
				ET2 = 1;
			}
			trefresh = 1;
			recalc_alarm = 1;
			lcd_display_onoff(1, 0, 0);
			break;

		case 0x0a:
			/* 切換顯示日期與時間的格式 */
			keydown = 1;
			dateType = !dateType;
			refresh = 1;
			break;
		}
		keydown = 1;
	}
}
