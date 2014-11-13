#ifndef USER_SPACE
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#else    /// USER_SPACE
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#endif   ///USER_SPACE

#include "bigmem.h"

#define PROC_NAME "bigmem"

struct big_mem g_mem;

/// 主函数定义
#ifndef USER_SPACE
struct proc_dir_entry *proc_file=NULL;
char *read_buf=NULL;
size_t temp=0;

ssize_t procfile_read(struct file* f,char *buf,size_t count,loff_t *offp)
{
	if(NULL==read_buf)
		return 0;
	if(count>temp)
		count=temp;
	temp-=count;
	copy_to_user(buf,read_buf,count);
	if(0==count)
		temp=strlen(read_buf);
	return count;
}

static const struct file_operations fops={
	.owner=THIS_MODULE,
	.read=procfile_read,
};

static int alloc_mem(struct big_mem *mem,size_t size)
{
	if(init_bigmem(mem,size,GFP_KERNEL|GFP_ATOMIC)<0)
	{
		printk("init bigmem failed\n");
		return -1;
	}
	if(dump_bigmem(mem,&read_buf)<0)
		read_buf=NULL;
	temp=strlen(read_buf);
	return 0;
}

static int write_mem(struct big_mem *mem)
{
	const char *hzy_str="hzy(hzy.oop@gmail.com";
	const int len=strlen(hzy_str);
	/// 清零
	if(set_bigmem(mem,0,get_bigmem_len(mem),0)<0)
	{
		printk("set_bigmem failed\n");
		return -1;
	}
	/// 写入内存
	if(write_bigmem(mem,get_bigmem_len(mem)-30,hzy_str,len)<0)
	{
		printk("write big_mem error\n");
		return -1;
	}
	return 0;
}

static int test_write_read(struct big_mem *mem)
{
	char buf[512];
	char buf_copy[512];
	int i=0;
	for(i=0;i<sizeof(buf)/sizeof(buf[0]);++i)
		buf[i]=(char)('a'+i%('z'-'a'));
	if(write_bigmem(mem,10,buf,sizeof(buf)/sizeof(buf[0]))<0)
	{
		printk("write_bigmem error\n");
		return -1;
	}
	if(read_bigmem(mem,10,buf_copy,sizeof(buf)/sizeof(buf[0]))<0)
	{
		printk("read_bigmem error\n");
		return -1;
	}
	if(memcmp(buf,buf_copy,sizeof(buf)/sizeof(buf[0]))==0)
		return 0;
	printk("read/write data not match\n");
	return -1;
}

static int create_proc_file(struct big_mem *mem)
{
	proc_file=proc_create(PROC_NAME,0444,NULL,&fops);
	if(NULL==proc_file)
	{
		printk("could not initialize /proc/%s",PROC_NAME);
		return -1;
	}
	return 0;
}

static int clean_proc_file(void)
{
	remove_proc_entry(PROC_NAME,NULL);
	return 0;
}

static int test_cmp(struct big_mem *mem)
{
	char buf[]="I am a test text";
	const int len=strlen(buf);
	int err=0;
	int res;
	if((err=write_bigmem_bh(mem,0,buf,sizeof(buf)))<0)
	{
		printk("write_bigmem_bh failed\n");
		return err;
	}
	if((err=cmp_bigmem(mem,0,buf,len,&res))<0)
	{
		printk("cmp_bigmem failed\n");
		return err;
	}
	if(res==0)
		return 0;
	return -1;
}

static int test_set(struct big_mem *mem)
{
	char data='a';
	int err=0;
	const int len=20;
	size_t start=4*1024*1024-10;
	int i=0;
	if((err=set_bigmem(mem,start,len,data))<0)
	{
		printk("set_bigmem failed\n");
		return err;
	}
	for(i=0;i<len;i++)
	{
		char value;
		if((err=read_bigmem(mem,start+i,&value,1))<0)
		{
			printk("read_bigmem failed\n");
			break;
		}
		if(data!=value)
			break;
	}
	if(i==len)
		return 0;
	return -1;
}

static int __init test_init(void)
{
	size_t size=5*1024*1024;
	if(alloc_mem(&g_mem,size)<0)
	{
		printk("alloc_mem failed\n");
		return -1;
	}
	if(write_mem(&g_mem)<0)
		printk("write_mem error\n");
	else
		printk("write mem id");
	printk("-----------------------\n");
	/// test code
	if(test_write_read(&g_mem)<0)
		printk("test_write_read error\n");
	else
		printk("test write_read ok\n");
	printk("-----------------------\n");
	if(test_set(&g_mem)<0)
		printk("test_set error\n");
	else
		printk("test set ok\n");
	printk("-----------------------\n");
	if(test_cmp(&g_mem)<0)
		printk("test_cmp error\n");
	else
		printk("test cmp ok\n");
	printk("-----------------------\n");

	if(create_proc_file(&g_mem)<0)
	{
		printk("create_proc_file failed\n");
		clean_bigmem(&g_mem);
		if(read_buf!=NULL)
		{
			kfree(read_buf);
			read_buf=NULL;
			temp=0;
		}
		return -1;
	}
	printk("catch the bigmem str: \n%s\n",read_buf);
	printk("create proc file /proc/%s ok\n",PROC_NAME);
	return 0;
}

static void __exit test_exit(void)
{
	if(NULL!=read_buf)
	{
		kfree(read_buf);
		temp=0;
	}
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
	int i=0;
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
			fprintf(fp,"(%d,%c)\n",i,data);
	}
}

int display_struct(struct big_mem *mem,FILE *fp)
{
	int i=0;
	if(NULL==mem)
		return -EINVAL;
	fprintf(fp,"count:%lu,size:%zu\n",mem->mem_count,mem->mem_size);
	if(NULL==mem->addrs||NULL==mem->sizes)
		return -EINVAL;
	for(i=0;i<mem->mem_count;i++)
		fprintf(fp,"0x%lx %zu\n",mem->addrs[i],mem->sizes[i]);
	return 0;
}

static int load_mem(struct big_mem *mem,const char *proc_name)
{
	char path[512]="/proc/";
	const int path_len=sizeof(path)/sizeof(path[0]);
	char buf[1024];
	const int buf_len=sizeof(buf)/sizeof(buf[0]);
	int i=0;
	int fd=0;
	int err=0;
	char *p=buf;
	int left_len=buf_len;

	strncat(path,PROC_NAME,path_len-strlen(path)-1);
	/// 读取proc内容
	FILE* fp;
	if((fp=fopen(path,"r"))==NULL)
	{
		error_at_line(0,errno,__FILE__,__LINE__,"fopen error");
		return -errno;
	}
	memset(buf,0,buf_len);
	while(left_len>1)
	{
		if(fgets(p,left_len-1,fp)==NULL)
			break;
		left_len-=strlen(p);
		p+=strlen(p);
	}
	fclose(fp);
	printf("catch the proc string:\n%s\n",buf);
	printf("--------------------------\n");
	/// 反序列化
	if((err=load_bigmem(mem,buf))<0)
	{
		error_at_line(0,errno,__FILE__,__LINE__,"load_bigmem failed");
		return err;
	}
	printf("display the struct:\n");
	display_struct(mem,stdout);
	printf("------------------------\n");
	/// 映射物理内存
	if((fd=open("/dev/mem",O_RDWR))<0)
	{
		error_at_line(0,errno,__FILE__,__LINE__,"open error");
		unmmap_clean_bigmem(mem);
		return -errno;
	}
	if(mmap_bigmem(mem,fd,PROT_READ|PROT_WRITE,MAP_SHARED)<0)
	{
		error_at_line(0,errno,__FILE__,__LINE__,"mmap_bigmem failed");
		unmmap_clean_bigmem(mem);
		return -errno;
	}
	return 0;
}

int main()
{
	int err=0;
	if((err=load_mem(&g_mem,PROC_NAME))<0)
	{
		error_at_line(0,-err,__FILE__,__LINE__,"load mem error\n");
		return -1;
	}
	printf("display the mem:\n");
	display_bigmem(&g_mem,stdout);
	unmmap_clean_bigmem(&g_mem);
	return 0;
}

#endif 	///USER_SPACE
