#include <8052.h>
#include "lcd.h"
#include "def.h"
#include "input.h"
#include "common.h"
#include "alarm.h"

__code char keycode[16] = { KEY_POSISION_0, KEY_POSISION_1, KEY_POSISION_2,
	KEY_POSISION_3, KEY_POSISION_4, KEY_POSISION_5,
	KEY_POSISION_6, KEY_POSISION_7, KEY_POSISION_8,
	KEY_POSISION_9, KEY_POSISION_A, KEY_POSISION_B,
	KEY_POSISION_C, KEY_POSISION_D, KEY_POSISION_E,
	KEY_POSISION_F
};

/* 用來取得現在所按下的按鍵 */
unsigned char get_keycode()
{
	unsigned char i, k;

	/* 取回輸入值用之變數 */
	unsigned char key;

	for (i = 0; i < 4; i++) {
		/* 輸出掃描訊號 */
		KEYPAD = ~(0x01 << (i + 4));

		/* 取回輸入值 */
		key = KEYPAD;
		for (k = 0; k < 4; k++) {
			key >>= 1;
			/* 偵測到按鍵 */
			if (!CY)
				return keycode[i << 2 | k];
		}
	}
	return 0xff;
}

/* 等待使用者按下按鍵，才回傳按鍵值 */
char wait_for_code()
{
	/* 存放按下之按鍵 */
	unsigned int code;
	__bit keydown = 1;

	/* 接收下一個按鍵放開 */
	while (1) {
		if ((code = get_keycode()) == 0xff) {
			keydown = 0;
			/* 進入省電模式 */
			PCON |= 1;

			/* 判斷是否要重新計算下次鈴響時間 */
			if (recalc_alarm) {
				calc_next();
				recalc_alarm = 0;
			}
			continue;
		}
		if (keydown)
			continue;
		return code;
	}
}

/* 輸入模式副程式，用來讓使用者輸入數字 */
unsigned int enter_number(char n, __bit first, unsigned char org,
			  __bit allow_enter, __bit auto_submit)
{
	/* 目前的數字 */
	unsigned int num = 0;

	/* 使用者輸入之數字 */
	unsigned char code = 0;

	char i = 0;

	/* 先清除要輸入的空格 */
	lcd_erase(0, n, 1);
	if (n > 5)
		n = 5;

	/* 是否包含第一個數字 */
	if (first) {
		num = org;
		lcd_putchar(org + '0');
		i = 1;
	}

	/* 進入等候輸入迴圈 */
	while (1) {

		/* 若輸入到最大值，且有自動送出時，自動回傳結果 
		 */
		if (i == n && auto_submit)
			return num;

		/* 等候一個數值 */
		while ((code = wait_for_code()) > 12) ;

		/* 
		 * 特殊功能鍵
		 * A: 送出
		 * B: 取消
		 * C: 擦去一個數字
		 */
		switch (code) {
		case 0x0a:
			/* 送出 */
			if (allow_enter)
				return num;
			else if (code == 10) {
				continue;
			}
		case 0x0b:
			/* 取消 */
			return 0xffff;
		case 0x0c:
			/* 擦去一個數字 */
			if (i != 0) {
				lcd_erase(1, 1, 1);
				num /= 10;
				i--;
			}
			continue;
		}

		/* 若會超過輸入限制，則不動作 */
		/* 
		 * 輸入限制為：最多輸入五位數，並限制最大為 65534
		 */
		if (i == n ||
		    (i > 3 && (num > 6553 || num == 6553 && code > 4))) {

			continue;
		}
		i++;

		/* 更新 num 變數 */
		num = num * 10 + code;
		lcd_putchar(code + '0');

	}
}
