/* I/O Port 設定 */
#define LCD_RW P1_1
#define LCD_RS P1_2
#define LCD_E  P1_0
#define LCD_D  P3
#define KEYPAD P2
#define BUZZ P0_0

/* 
 * 鬧鐘相關設定
 * ALARM_CLOCK_NUMBER 鬧鐘數量
 * MAX_RING 最大鈴響秒數
 * MAX_SNOOZING 最大賴床秒數，也就是按下頼床鍵的秒數
 */
#define ALARM_CLOCK_NUMBER 20
#define MAX_RING 3600
#define MAX_SNOOZING 300

/* 
 * 頻率調整
 * CLOCK_D12 頻率除以 12
 * CLOCK_LEAP 每次閏百分之一秒計數
 * CLOCK_LEAP_MAX 計數超過此數字則閏百分之一秒
 */
#define CLOCK_D12   9215
#define CLOCK_LEAP 1
#define CLOCK_LEAP_MAX 88465

/* 
 * 出題最大值 (M_MAX)與最小值 (M_MIN)
 * 最大可設為為 65534 ，且 M_MAX 必須比 M_MIN 之 2 倍大
 * 此數值會在題目與答案表現出來
 */
#define M_MAX 999
#define M_MIN 50

/* 按鍵位置 */
#define KEY_POSISION_0 7
#define KEY_POSISION_1 8
#define KEY_POSISION_2 9
#define KEY_POSISION_3 12
#define KEY_POSISION_4 4
#define KEY_POSISION_5 5
#define KEY_POSISION_6 6
#define KEY_POSISION_7 13
#define KEY_POSISION_8 1
#define KEY_POSISION_9 2
#define KEY_POSISION_A 3
#define KEY_POSISION_B 14
#define KEY_POSISION_C 0
#define KEY_POSISION_D 10
#define KEY_POSISION_E 11
#define KEY_POSISION_F 15

/* 星期位移 */
#define WEEK_OFFSET 6
