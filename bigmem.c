#ifndef USER_SPACE
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#else    /// USER_SPACE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#endif   ///USER_SPACE

#include "bigmem.h"

#define AUTHOR "hzy(hzy.oop@gmail.com)"
#define DESCRIPTION "a big mem data structure"
#define VERSION "1.0"


/// @brief 依据内存index计算，内存单元所在的块号和，块内索引
/// @retval 0成功 <0失败
static int cal_bigmem_coord(struct big_mem *mem,size_t index,unsigned long *block_index,size_t *inner_index)
{
	int i=0;
	size_t sum=0;
	if(mem==NULL)
		return -EINVAL;
	if(index>=mem->mem_size)
		return -ENOMEM;
	/// 计算块号
	for(i=0;i<mem->mem_count;i++)
	{
		size_t end;
		end=sum+mem->sizes[i];
		if(sum<=index&&end>index)
			break;
		sum=end;
	}
	/// 判断是否越界
	if(i==mem->mem_count)
		return -ENOMEM;
	if(NULL!=block_index)
		*block_index=i;
	if(inner_index!=NULL)
		*inner_index=index-sum;
	return 0;
}

#ifndef USER_SPACE
/// @brief 依据大小计算内存块数,及末尾块大小
/// @param[out] count内存块个数
/// @param[out] size 未块的大小
/// @retval 0成功, -1失败
static int mem_count(size_t mem_size,unsigned long *mem_count,size_t *block_size)
{
	int order=0;
	int count=0;   ///< 内存块数
	size_t size=0;   ///< 内存末块的大小
	/// mem_size非法参数
	if(mem_size==0)
		return -EINVAL;
	/// 计算count,size
	order=get_order(mem_size);

	count=order-BIGMEM_MAX_ORDER>0?1<<(order-BIGMEM_MAX_ORDER):1;
	size=count==1?mem_size:mem_size-(1<<BIGMEM_MAX_ORDER)*PAGE_SIZE;
	if(count>BIGMEM_MAX_COUNT)
		return -ENOMEM;
	///返回
	if(NULL!=mem_count)
		*mem_count=count;
	if(NULL!=block_size)
		*block_size=size;
	return 0;
}
#endif  /// USER_SPACE

static int _write_bigmem(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size)
{
	unsigned long block_index0,block_index1;  ///< 起止内存块索引
	size_t inner_index0,inner_index1;    ///< 起止内存内索引
	size_t end=begin+buf_size-1;
	int err=0;
	if(NULL==mem)
		return -EINVAL;
	if((err=cal_bigmem_coord(mem,begin,&block_index0,&inner_index0))<0)
		return err;
	if((err=cal_bigmem_coord(mem,end,&block_index1,&inner_index1))<0)
		return err;
	/// 禁止跨越两个内存块
	if(block_index1-block_index0>1)
		return -EINVAL;
	/// 内存拷贝
	if(block_index1==block_index0)
		memcpy((void*)(mem->addrs[block_index0]+inner_index0),buf,buf_size);
	else
	{
		size_t len=mem->sizes[block_index0]-inner_index0;
		memcpy((void*)(mem->addrs[block_index0]+inner_index0),buf,len);
		memcpy((void*)(mem->addrs[block_index1]),buf+len,buf_size-len);
	}
	return 0;
}

/// @brief 读取数据到缓冲区
static int _read_bigmem(struct big_mem *mem,size_t begin,void *buf,size_t buf_size)
{
	unsigned long block_index0,block_index1;  ///< 起止内存块索引
	size_t inner_index0,inner_index1;    ///< 起止内存内索引
	size_t end=begin+buf_size-1;
	int err=0;
	if(NULL==mem)
		return -EINVAL;
	/// 计算起止地址内存坐标
	if((err=cal_bigmem_coord(mem,begin,&block_index0,&inner_index0))<0)
		return err;
	if((err=cal_bigmem_coord(mem,end,&block_index1,&inner_index1))<0)
		return err;
	/// 禁止间隔两个内存块的读取
	if(block_index1-block_index0>1)
		return -EINVAL;
	/// 复制数据到buf
	if(block_index0==block_index1)
		memcpy(buf,(void*)(mem->addrs[block_index0]+inner_index0),buf_size);
	else
	{
		size_t len=mem->sizes[block_index0]-inner_index0;
		memcpy(buf,(void*)(mem->addrs[block_index0]+inner_index0),len);
		memcpy(buf+len,(void*)(mem->addrs[block_index1]),buf_size-len);
	}
	return err;
}

/// @brief 设置内存数据为data
static int _set_bigmem(struct big_mem *mem,size_t begin,size_t len,char data)
{
	unsigned long block_index0,block_index1;  ///< 起止内存块索引
	size_t inner_index0,inner_index1;    ///< 起止内存内索引
	size_t end=begin+len-1;
	int err=0;
	int i=0;

	if(NULL==mem)
		return -EINVAL;
	if((err=cal_bigmem_coord(mem,begin,&block_index0,&inner_index0))<0)
		return err;
	if((err=cal_bigmem_coord(mem,end,&block_index1,&inner_index1))<0)
		return err;
	/// 设置内存值
	if(block_index0==block_index1)
	{
		memset((void*)(mem->addrs[block_index0]+inner_index0),data,inner_index1-inner_index0+1);
		return 0;
	}
	for(i=block_index0;i<=block_index1;i++)
	{
		size_t len=0;
		if(i==block_index0)
			len=mem->sizes[block_index0]-inner_index0;
		else if(i==block_index1)
			len=inner_index1+1;
		else
			len=mem->sizes[i];

		if(i==block_index0)
			memset((void*)(mem->addrs[i]+inner_index0),data,len);
		else
			memset((void*)(mem->addrs[i]),data,len);
	}
	return 0;
}

/// @brief 对比内存缓冲区的数据
/// @param[out] res比较结果
static int _cmp_bigmem(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size,int *res)
{
	unsigned long block_index0,block_index1;  ///< 起止内存块索引
	size_t inner_index0,inner_index1;    ///< 起止内存内索引
	size_t end=begin+buf_size-1;
	int err=0;
	int i=0;

	if(NULL==mem)
		return -EINVAL;
	if(NULL==res)
		return -EINVAL;
	if((err=cal_bigmem_coord(mem,begin,&block_index0,&inner_index0))<0)
		return err;
	if((err=cal_bigmem_coord(mem,end,&block_index1,&inner_index1))<0)
		return err;
	/// 内存对比
	if(block_index0==block_index1)
		*res=memcmp(buf,(void*)(mem->addrs[block_index0]+inner_index0),buf_size);
	else
	{
		for(i=block_index0;i<=block_index0;i++)
		{
			size_t len;
			if(i==block_index0)
				len=mem->sizes[i]-inner_index0;
			else if(i==block_index1)
				len=inner_index1+1;
			else
				len=mem->sizes[i];
			
			if(i==block_index0)
			{
				if((*res=memcmp((void*)(mem->addrs[i]+inner_index0),buf,len))!=0)
					break;
			}
			else
			{
				if((*res=memcmp((void*)(mem->addrs[i]),buf,len))!=0)
					break;
			}
			buf+=len;
		}
	}
	return err;
}

#ifndef USER_SPACE
/// @brief 初始化bigmem结构
/// @param[in] size 内存大小
/// @retval 0成功,<0失败
int init_bigmem(struct big_mem *mem,size_t mem_size,gfp_t flags)
{
	int err=0;   ///< 错误码
	unsigned long block_count;   ///< 内存块
	size_t end_size;   ///< 末块大小
	int i;
	int mem_index=0;
	
	/// 判断参数
	if(NULL==mem)
		return -EINVAL;
	if((err=mem_count(mem_size,&block_count,&end_size))<0)
		return err;
	if(NULL==mem)
		return -EINVAL;
	/// 初始化内存块
	err=0;
	mem->mem_size=mem_size;
	mem->mem_count=block_count;
	mem->addrs=(unsigned long*)kmalloc(sizeof(unsigned long)*mem->mem_count,GFP_KERNEL|GFP_ATOMIC);
	mem->sizes=(size_t*)kmalloc(sizeof(size_t)*mem->mem_count,GFP_KERNEL|GFP_ATOMIC);
	if(NULL==mem->addrs||NULL==mem->sizes)
	{
		err=-ENOMEM;
		goto clean_addrs;
	}
	/// 分配内存
	for(mem_index=0;mem_index<mem->mem_count-1;mem_index++)
	{
		if((mem->addrs[mem_index]=__get_free_pages(flags,BIGMEM_MAX_ORDER))==0)
		{
			err=-ENOMEM;
			goto clean_pages;
		}
		mem->sizes[mem_index]=(1<<BIGMEM_MAX_ORDER)*PAGE_SIZE;
	}
	mem->addrs[mem->mem_count-1]=__get_free_pages(flags,get_order(end_size));
	mem_index++;
	mem->sizes[mem->mem_count-1]=end_size;
	if(mem->addrs[mem->mem_count-1]==0)
	{
		err=-ENOMEM;
		goto clean_pages;
	}
	/// 初始化锁
	rwlock_init(&mem->lock);
	return err;
clean_pages:
	for(i=0;i<mem_index;i++)
	{
		if(i==mem->mem_count-1)
		{
			free_pages(mem->addrs[i],get_order(end_size));
			mem->addrs[i]=0;
		}
		else
		{
			free_pages(mem->addrs[i],BIGMEM_MAX_ORDER);
			mem->addrs[i]=0;
		}
	}
clean_addrs:
	if(mem->addrs!=NULL)
		kfree(mem->addrs);
	if(mem->sizes!=NULL)
		kfree(mem->sizes);
	mem->addrs=NULL;
	mem->sizes=NULL;
	return err;
}
EXPORT_SYMBOL(init_bigmem);

/// @brief 清除bigmem结构
void clean_bigmem(struct big_mem *mem)
{
	int i=0;
	if(NULL==mem)
		return;
	/// 释放内存
	for(i=0;i<mem->mem_count;i++)
	{
		if(i==mem->mem_count-1)
			free_pages(mem->addrs[i],get_order(mem->sizes[i]));
		else
			free_pages(mem->addrs[i],BIGMEM_MAX_ORDER);
	}
	/// 释放块数组
	kfree(mem->addrs);
	kfree(mem->sizes);
	mem->addrs=mem->sizes=NULL;
}
EXPORT_SYMBOL(clean_bigmem);
#endif   /// USER_SPACE


/// @brief 把缓冲区数据写入内存
/// @param[in] begin,写入到mem的位置
/// @param[in] buf,缓冲区首地址
/// @param[in] size, 缓冲区大小
/// @retval 0成功, <0失败
int write_bigmem(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size)
{
	int err=0;
	if(NULL==mem)
		return -EINVAL;
#ifndef USER_SPACE
	write_lock(&mem->lock);
#endif
	err=_write_bigmem(mem,begin,buf,buf_size);
#ifndef USER_SPACE
	write_unlock(&mem->lock);
#endif
	return err;
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
	int err=0;
	if(NULL==mem)
		return -EINVAL;
#ifndef USER_SPACE
	read_lock(&mem->lock);
#endif
	err=_read_bigmem(mem,begin,buf,buf_size);
#ifndef USER_SPACE
	read_unlock(&mem->lock);
#endif
	return err;
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
	int err=0;
	if(NULL==mem)
		return -EINVAL;
#ifndef USER_SPACE
	write_lock(&mem->lock);
#endif
	err=_set_bigmem(mem,begin,len,data);
#ifndef USER_SPACE
	write_unlock(&mem->lock);
#endif
	return err;
}
#ifndef USER_SPACE
EXPORT_SYMBOL(set_bigmem);
#endif


/// @brief 返回内存数据的长度
size_t get_bigmem_len(const struct big_mem *mem)
{
	if(NULL==mem)
		return 0;
	return mem->mem_size;
}

#ifndef USER_SPACE
EXPORT_SYMBOL(get_bigmem_len);
#endif

/// @breif 对比缓冲区和内存结构中的数据
/// @param[in] begin,对比数据的首地址
/// @param[in] buf,缓冲区首地址
/// @param[in] size, 缓冲区大小
/// @param[out] res memcmp的返回值
/// @retval 0成功 <0失败
int cmp_bigmem(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size,int *res)
{
	int err=0;
	if(NULL==mem)
		return -EINVAL;
#ifndef USER_SPACE
	read_lock(&mem->lock);
#endif
	err=_cmp_bigmem(mem,begin,buf,buf_size,res);
#ifndef USER_SPACE
	read_unlock(&mem->lock);
#endif
	return 0;
}
#ifndef USER_SPACE
EXPORT_SYMBOL(cmp_bigmem);
#endif

#ifndef USER_SPACE
/// @brief 把缓冲区数据写入内存,使用bh锁
int write_bigmem_bh(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size)
{
	int err=0;
	if(NULL==mem)
		return -EINVAL;
	write_lock_bh(&mem->lock);
	err=_write_bigmem(mem,begin,buf,buf_size);
	write_unlock_bh(&mem->lock);
	return err;
}
EXPORT_SYMBOL(write_bigmem_bh);
/// @brief 读取数据到缓冲区 使用bh锁
int read_bigmem_bh(struct big_mem *mem,size_t begin,void *buf,size_t buf_size)
{
	int err=0;
	if(NULL==mem)
		return -EINVAL;
	read_lock_bh(&mem->lock);
	err=_read_bigmem(mem,begin,buf,buf_size);
	read_unlock_bh(&mem->lock);
	return err;
}
EXPORT_SYMBOL(read_bigmem_bh);
/// @brief 设置内存数据为data
int set_bigmem_bh(struct big_mem *mem,size_t begin,size_t len,char data)
{
	int err=0;
	if(NULL==mem)
		return -EINVAL;
	write_lock_bh(&mem->lock);
	err=_set_bigmem(mem,begin,len,data);
	write_unlock_bh(&mem->lock);
	return err;
}
EXPORT_SYMBOL(set_bigmem_bh);

/// @breif 对比缓冲区和内存结构中的数据
/// @param[out] res memcmp的返回值
/// @retval 0成功 <0失败
int cmp_bigmem_bh(struct big_mem *mem,size_t begin,const void *buf,size_t buf_size,int *res)
{
	int err=0;
	if(NULL==mem)
		return -EINVAL;
	read_lock_bh(&mem->lock);
	err=_cmp_bigmem(mem,begin,buf,buf_size,res);
	read_unlock_bh(&mem->lock);
	return err;
}
EXPORT_SYMBOL(cmp_bigmem_bh);
#endif   ///USER_SPACE

#ifndef USER_SPACE
/// @brief 将big_mem数据写入proc文件
int dump_bigmem(struct big_mem *mem,char **strdata)
{
	const int STR_LEN=512;
	int err=0;
	if(NULL==mem||NULL==strdata)
		return -EINVAL;
	/// 分配内存
	*strdata=(char*)kmalloc(STR_LEN,GFP_ATOMIC|GFP_KERNEL);
	if(*strdata==NULL)
		return -ENOMEM;
	/// 序列化到字符串
	do
	{
		int i=0;
		char *str=*strdata;
		size_t len=snprintf(str,STR_LEN,"%lu %zu\n",mem->mem_count,mem->mem_size);
		if(len>=STR_LEN)
		{
			err=-ENOMEM;
			break;
		}
		str=*strdata+len;
		for(i=0;i<mem->mem_count;i++)
		{
			len+=snprintf(str,STR_LEN-len,"0x%lx %zu\n",(unsigned long)virt_to_phys((char*)(mem->addrs[i])),mem->sizes[i]);
			if(len>=STR_LEN)
			{
				err=-ENOMEM;
				break;
			}
			str=*strdata+len;
		}
	}
	while(0);
	if(err!=0)
		kfree(*strdata);
	return err;
}
EXPORT_SYMBOL(dump_bigmem);
#else    /// USER_SAPCE
/// @brief 将字符串反序列化为big_mem
int load_bigmem(struct big_mem *mem,const char *strdata)
{
	int err=0;
	int i;
	if(NULL==mem||NULL==strdata)
		return -EINVAL;
	/// 复制字符串
	size_t len=strlen(strdata)+1;
	char *buf=(char*)malloc(len);
	char *saveptr;
	if(NULL==buf)
		return -ENOMEM;
	strncpy(buf,strdata,len);
	buf[len-1]='\0';
	/// 反序列化mem_count,mem_size
	char *tok=strtok_r(buf,"\n",&saveptr);
	if(tok==NULL)
	{
		err=-EINVAL;
		goto free_buf;
	}
	if(sscanf(tok,"%lx %zu",&mem->mem_count,&mem->mem_size)!=2)
	{
		err=-EINVAL;
		goto free_buf;
	}
	/// 分配mem->addrs,mem->sizes
	mem->addrs=(unsigned long*)malloc(sizeof(unsigned long)*mem->mem_count);
	mem->sizes=(size_t*)malloc(sizeof(size_t)*mem->mem_count);
	if(NULL==mem->addrs||NULL==mem->sizes)
		goto free_mem;
	/// 反序列化 addrs sizes
	for(i=0;i<mem->mem_count;i++)
	{
		tok=strtok_r(NULL,"\n",&saveptr);
		if(NULL==tok)
		{
			err=-EINVAL;
			goto free_mem;
		}
		if(sscanf(tok,"%lx %lu",mem->addrs+i,mem->sizes+i)!=2)
		{
			err=-EINVAL;
			goto free_mem;
		}
	}
	return 0;
free_mem:
	if(mem->addrs!=NULL)
		free(mem->addrs);
	if(mem->sizes!=NULL)
		free(mem->sizes);
free_buf:
	free(buf);
	return err;
}
#endif  /// USER_SPACE

#ifdef USER_SPACE

/// @breif 读取内存设备文件，映射bigmem结构
/// @retval 0成功 <0失败(错误代码的负值)
int mmap_bigmem(struct big_mem *mem,int fd,int port,int flags)
{
	int err=0;
	int i=0;
	int map_index=0;
	if(NULL==mem||mem->addrs==NULL||NULL==mem->sizes)
		return -EINVAL;
	/// 映射文件
	for(map_index=0;map_index<mem->mem_count;++map_index)
	{
		void *buf=mmap(NULL,mem->sizes[map_index],port,flags,fd,mem->addrs[map_index]);
		if(MAP_FAILED==buf)
		{
			err=-errno;
			goto free_mmap;
		}
		mem->addrs[map_index]=(unsigned long)buf;
	}
	return 0;
free_mmap:
	for(i=0;i<map_index;i++)
	{
		munmap((void*)mem->addrs[i],mem->sizes[i]);
		mem->addrs[i]=0;
	}
	return err;

}

/// @brief 取消内存设备的映射,并释放bigmem的内存
/// @retval 0 成功 <0失败
int unmmap_clean_bigmem(struct big_mem *mem)
{
	int i=0;
	int err=0;
	if(NULL==mem||NULL==mem->addrs||NULL==mem->sizes)
		return -EINVAL;
	/// 取消映射的内存
	for(i=0;i<mem->mem_count;++i)
		if(mem->addrs&&mem->addrs[i]!=0)
		{
			munmap((void*)mem->addrs[i],mem->sizes[i]);
			mem->addrs[i]=0;
		}
	/// 释放内存
	free(mem->sizes);
	free(mem->addrs);
	mem->sizes=NULL;
	mem->addrs=NULL;
}

#endif

#ifndef USER_SPACE
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
#endif /// USER_SPACE
