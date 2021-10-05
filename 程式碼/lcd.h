/* LCD 2x20 顯示器常用功能 */

void lcd_putchar(char c);
void lcd_set_line(__bit line, char *str);
void lcd_set(__bit RS, unsigned char D, char busy_check);
void lcd_init();
void lcd_display_onoff(__bit D, __bit C, __bit B);
void lcd_cursor(__bit line, unsigned char pos);
void lcd_clear_line(__bit n);
void lcd_show_num(unsigned int num, unsigned char n, __bit zero,
		  __bit padding) __reentrant;
void lcd_puts(char *str) __reentrant;
void lcd_erase(__bit before, char n, __bit ret);
