#include "common.h"
#include "def.h"

/* 延遲一段時間 */
void delay(unsigned int time)
{
	while (time--) ;
}

/* x 的 y 次方 */
unsigned int pow(unsigned int x, unsigned int y)
{
	unsigned int i;
	unsigned int num = 1;

	for (i = 1; i <= y; i++)
		num *= x;
	return num;
}
