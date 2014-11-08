#ifndef BIG_MEM_H
#define BIG_MEM_H

#ifndef USER_SPACE

#include <linux/rwlock_types.h>
#include <linux/slab.h>
#define BIGMEM_MAX_ORDER 10    ///< 每次分配的最大order值
#define BIGMEM_MAX_COUNT 32    ///< 分配的最大内存块个数

#endif /// USER_SPACE

struct big_mem
{
	unsigned long *addrs;   ///< 内存块首地址数组
	size_t *sizes;   ///< 内存块大小数组
	unsigned long mem_count;  ///< 内存块数量
	size_t mem_size;  ///< 内存大小
#ifndef USER_SPACE
	rwlock_t lock;          ///< 锁
#endif   /// USER_SPACE
};


#ifndef USER_SPACE
/// @brief 初始化bigmem结构
/// @param[in] size 内存大小
/// @retval 0成功,<0失败
void init_bigmem(struct big_mem *mem,size_t size,gfp_t flags);
/// @brief 清除bigmem结构
/// @param[in] size 内存大小
/// @retval 0成功,<0失败
int clean_bigmem(struct big_mem *mem);
#endif   /// USER_SPACE

/// @brief 把缓冲区数据写入内存
/// @param[in] begin,写入到mem的位置
/// @param[in] buf,缓冲区首地址
/// @param[in] size, 缓冲区大小
/// @retval 0成功, <0失败
int write_bigmem(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size);

/// @brief 读取数据到缓冲区
/// @param[in] begin,读取数据的mem的位置
/// @param[in] buf,缓冲区首地址
/// @param[in] size, 缓冲区大小
/// @retval 0成功, <0失败
int read_bigmem(struct big_mem *mem,size_t begin,void *buf,size_t buf_size);

/// @brief 设置内存数据为data
/// @param[in] begin,len设置内存的范围(起始，长度)
/// @param[in] data,设置为的值
/// @retval 0成功, <0失败
int set_bigmem(struct big_mem *mem,size_t begin,size_t len,char data);

/// @brief 返回内存数据的长度
int get_bigmem_len(const struct big_mem *mem);

/// @breif 对比缓冲区和内存结构中的数据
/// @param[in] begin,对比数据的首地址
/// @param[in] buf,缓冲区首地址
/// @param[in] size, 缓冲区大小
/// @retval 参考memcmp的返回值
int cmp_bigmem(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size);

#ifndef USER_SPACE
/// @brief 将big_mem数据写入proc文件
int dump_bigmem_proc(struct big_mem *mem,const char *proc_path);
#else    /// USER_SAPCE
/// @brief 将big_mem从proc中读出
int load_bigmem_proc(struct big_mem *mem,const char *proc_path);
#endif  /// USER_SPACE

#endif  //BIG_MEM_H
