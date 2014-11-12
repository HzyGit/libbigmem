#ifndef USER_SPACE
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#else    /// USER_SPACE
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#endif   ///USER_SPACE

#include "bigmem.h"

/// 主函数定义
#ifndef USER_SPACE
static int __init test_init(void)
{
	return 0;
}
static void __exit test_exit(void)
{
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("a test for bigmem");
MODULE_AUTHOR("hzy(hzy.oop@gmail.com");
#else   ///USER_SPACE

int main()
{
}
#endif 	///USER_SPACE
