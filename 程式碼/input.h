/* 各函數功能請參閱 input.c */
unsigned char get_keycode();
unsigned char keycode_to_num(char code);
char wait_for_code();
unsigned char num_to_keycode(char code);
unsigned int enter_number(char n, __bit first, unsigned char org,
			  __bit allow_enter, __bit auto_submit);
