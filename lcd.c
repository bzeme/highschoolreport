#include <8052.h>
#include "lcd.h"
#include "def.h"
#include "common.h"

/* 輸出一個字元 */
void lcd_putchar(char c)
{
	lcd_set(1, c, 1);
}

/* 設定一列字串 */
void lcd_set_line(__bit line, char *str)
{
	lcd_clear_line(line);
	lcd_cursor(line, 0);
	lcd_puts(str);
}

/* 輸出字串 */
void lcd_puts(char *str) __reentrant
{
	char i;

	for (i = 0;; i++) {
		if (*(str + i) == '\0')
			break;
		lcd_putchar(*(str + i));
	}
}

/* 檢查忙錄旗標 */
void wait_lcd()
{
	LCD_RS = 0;
	LCD_RW = 1;
	LCD_D = 0xff;
	LCD_E = 1;
	while (LCD_D >> 7) {

		LCD_E = 0;
		LCD_E = 1;
	}
	LCD_E = 0;
	LCD_D = 0xff;
}

/* 傳送命令給 LCD */
void lcd_set(__bit RS, unsigned char D, char busy_check)
{
	if (busy_check)
		wait_lcd();

	LCD_RS = RS;
	LCD_RW = 0;
	LCD_E = 1;
	LCD_D = D;
	LCD_E = 0;
	LCD_D = 0xff;
}

/* LCD 顯示器開關 */
void lcd_display_onoff(__bit D, __bit C, __bit B)
{
	lcd_set(0, 0x08 | (D << 2) | (C << 1) | B, 1);
}

/* LCD 移動游標 */
void lcd_cursor(__bit line, unsigned char pos)
{
	lcd_set(0, 0x80 + line * 0x40 + pos, 1);
}

/* LCD 初始化 */
void lcd_init()
{

	/* 初始化 LCD */
	delay(2000);
	lcd_set(0, 0x30, 0);
	delay(500);
	lcd_set(0, 0x30, 0);
	delay(15);
	lcd_set(0, 0x30, 0);
	lcd_set(0, 0x38, 1);
	lcd_set(0, 0x08, 1);
	lcd_set(0, 0x01, 1);
	lcd_set(0, 0x06, 1);
	lcd_set(0, 0x0c, 1);
}

/* 清除一行 */
void lcd_clear_line(__bit n)
{
	lcd_cursor(n, 0);
	lcd_erase(0, 20, 1);
}

/* 顯示數字 */
void lcd_show_num(unsigned int num, unsigned char n, __bit zero,
		  __bit padding) __reentrant
{
	unsigned char i;
	unsigned char num2;
	__bit non_zero = 0;

	for (i = 1; i <= n; i++) {
		/* 計算要顯示的數字 */
		num2 = ((num / pow(10, n - i) % 10));

		/* 若為非 0 的數字 */
		if (num2 != zero)
			non_zero = 1;

		if (!non_zero && !zero && i != n) {
			/* 若高位數為 0 時 */
			if (padding)
				lcd_putchar(' ');	/* 補空格 */
			continue;
		}
		lcd_putchar('0' + num2);
	}
}

/* 清除文字 */
void lcd_erase(__bit before, char n, __bit ret)
{
	char i;

	if (before) {
		/* 要清除游標前的數字 */
		for (i = 0; i < n; i++)
			lcd_set(0, 0x10, 1);
	}
	for (i = 0; i < n; i++)
		lcd_putchar(' ');
	if (ret)
		for (i = 0; i < n; i++)
			lcd_set(0, 0x10, 1);

}
