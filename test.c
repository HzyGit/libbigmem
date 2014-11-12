#ifndef USER_SPACE
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#else    /// USER_SPACE
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <error.h>
#endif   ///USER_SPACE

#include "bigmem.h"

#define PROC_MEM "bigmem"

struct big_mem g_mem;
/// 主函数定义
#ifndef USER_SPACE

static int alloc_mem(struct big_mem *mem,size_t size)
{
	return -1;
}

static int write_mem(struct big_mem *mem,const void *buf,size_t len)
{
	return -1;
}
static int test_write_read(struct big_mem *mem)
{
	return -1;
}

static int create_proc_file(struct big_mem *mem)
{
	return -1;
}

static int clean_proc_file(void)
{
	return -1;
}

static int test_set_cmp(struct big_mem *mem)
{
	return -1;
}

static int __init test_init(void)
{
	size_t size=1024;
	const char *buf="hello, I am a test text";
	if(alloc_mem(&g_mem,size)<0)
	{
		printk("alloc_mem failed\n");
		return -1;
	}
	if(write_mem(&g_mem,buf,strlen(buf))<0)
		printk("write_mem error\n");
	if(test_write_read(&g_mem)<0)
		printk("test_write_read error");
	if(test_set_cmp(&g_mem)<0)
		printk("test_set_mem error\n");
	if(create_proc_file(&g_mem)<0)
	{
		printk("create_proc_file failed\n");
		clean_bigmem(&g_mem);
		return -1;
	}
	return 0;
}

static void __exit test_exit(void)
{
	clean_proc_file();
	clean_bigmem(&g_mem);
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("a test for bigmem");
MODULE_AUTHOR("hzy(hzy.oop@gmail.com");
#else   ///USER_SPACE

#define	MEM_DUMP_FILE "tes.mem.txt"
int display_bigmem(struct big_mem *mem,FILE *fp)
{
	int err=0;
	size_t i=0;
	if(NULL==mem||NULL==fp)
		return 0;
	for(i=0;i<get_bigmem_len(mem);++i)
	{
		char data;
		if((err=read_bigmem(mem,i,&data,1))<0)
		{
			error_at_line(0,-err,__FILE__,__LINE__,"read_bigmem error");
			return err;
		}
		if(data!=0)
			fprintf(fp,"(%d,%d)\n",i,data);
	}
}

int main()
{
}

#endif 	///USER_SPACE
