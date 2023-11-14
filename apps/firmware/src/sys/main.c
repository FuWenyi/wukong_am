#include <global.h>
#include <register.h>

extern void test_ut_main(void);


int main(void)
{
    test_ut_main();     // 测试 sram / uart / spi
    return 0;
}
