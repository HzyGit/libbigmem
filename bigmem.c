#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "bigmem.h"

#define AUTHOR "hzy(hzy.oop@gmail.com)"
#define DESCRIPTION "a big mem data structure"
#define VERSION "1.0"


#ifndef USER_SPACE
/// @brief 初始化bigmem结构
/// @param[in] size 内存大小
/// @retval 0成功,<0失败
int init_bigmem(struct big_mem *mem,size_t size,gfp_t flags)
{
	if(NULL==mem)
		return -EINVAL;
	return 0;
}
EXPORT_SYMBOL(init_bigmem);
#endif   /// USER_SPACE


/// @brief 把缓冲区数据写入内存
/// @param[in] begin,写入到mem的位置
/// @param[in] buf,缓冲区首地址
/// @param[in] size, 缓冲区大小
/// @retval 0成功, <0失败
int write_bigmem(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size)
{
	return 0;
}
#ifndef USER_SPACE
EXPORT_SYMBOL(write_bigmem);
#endif

/// @brief 读取数据到缓冲区
/// @param[in] begin,读取数据的mem的位置
/// @param[in] buf,缓冲区首地址
/// @param[in] size, 缓冲区大小
/// @retval 0成功, <0失败
int read_bigmem(struct big_mem *mem,size_t begin,void *buf,size_t buf_size)
{
	return 0;
}
#ifndef USER_SPACE
EXPORT_SYMBOL(read_bigmem);
#endif


/// @brief 设置内存数据为data
/// @param[in] begin,len设置内存的范围(起始，长度)
/// @param[in] data,设置为的值
/// @retval 0成功, <0失败
int set_bigmem(struct big_mem *mem,size_t begin,size_t len,char data)
{
	return 0;
}
#ifndef USER_SPACE
EXPORT_SYMBOL(set_bigmem);
#endif


/// @brief 返回内存数据的长度
int get_bigmem_len(const struct big_mem *mem)
{
	return 0;
}
#ifndef USER_SPACE
EXPORT_SYMBOL(get_bigmem_len);
#endif

/// @breif 对比缓冲区和内存结构中的数据
/// @param[in] begin,对比数据的首地址
/// @param[in] buf,缓冲区首地址
/// @param[in] size, 缓冲区大小
/// @retval 参考memcmp的返回值
int cmp_bigmem(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size)
{
	return 0;
}
#ifndef USER_SPACE
EXPORT_SYMBOL(cmp_bigmem);
#endif

#ifndef USER_SPACE
/// @brief 将big_mem数据写入proc文件
int dump_bigmem_proc(struct big_mem *mem,const char *proc_path)
{
	return 0;
}
EXPORT_SYMBOL(dump_bigmem_proc);
#else    /// USER_SAPCE
/// @brief 将big_mem从proc中读出
int load_bigmem_proc(struct big_mem *mem,const char *proc_path)
{
	return 0;
}
#endif  /// USER_SPACE

/// @brief 依据大小计算内存块数,及末尾块大小
/// @param[out] count内存块个数
/// @param[out] size 未块的大小
/// @retval 0成功, -1失败
static int mem_count(size_t mem_size,unsigned long *count,size_t size)
{
	int order=0;
	/// mem_size非法参数
	if(mem_size==0)
		return -EINVAL;
	order=get_order(mem_size);
	return 0;
}

static int __init init_bigmem_module(void)
{
	printk(KERN_INFO "bigmem(v%s) module load ok!\n",VERSION);
	return 0;
}

static void __exit cleanup_bigmem_module(void)
{
	printk(KERN_INFO "bigmem(v%s) module unload ok!\n",VERSION);
}


module_init(init_bigmem_module);
module_exit(cleanup_bigmem_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

