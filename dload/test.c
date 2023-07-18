#include <stdio.h>
#include "compiler.h"


static int x = 0;
void inter(void){ x = 1; }

/* 链接脚本中将该函数放置于头部位置 */
SECTION(".text.startup") void test(void)
{
    inter();
    printf("testing!!!!%x\n", x);
}