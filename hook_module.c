#include <linux/init.h>     
#include <linux/init_task.h>  
#include <linux/module.h>      
#include <linux/kernel.h> 	   
#include <linux/unistd.h>	  	   
#include <linux/sched.h>     
#define my_syscall_num 300     //新增的300号系统功能调用
#define sys_call_table_address 0xffffffff92000280   //sys_call_table的地址
#define MAX_PROCESS_AMOUNT 1024

//进程节点
struct process_tree_node{
	int pid;   
	int depth; 
};
//进程树结构
struct process_tree{
    struct process_tree_node processes[MAX_PROCESS_AMOUNT];
    int cnt;
};
int make_rw(unsigned long address);//取消写保护
int make_ro(unsigned long address); //写保护

unsigned long * sys_call_table; //指向sys_call_table首地址
unsigned long saved; //保存原始数据

struct process_tree tree; //进程树

/* make the page writable */
int make_rw(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);//查找虚拟地址address所对应的物理页表项
	pte->pte |= _PAGE_RW;//设置页表读写属性
	return 0; 
	//页表项pte->pte是一个unsigned long 类型，其低12bit存储该物理页的属性信息，
	//其余位存储物理页框号pfn(page frame number)，即物理基地址，加上12bit的offset就是实际物理地址
}
/* make the page write protected */
int make_ro(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte &= ~_PAGE_RW;//设置只读属性
	return 0;
}
//自定义的300号系统调用
void process_traverse_helper(struct task_struct *p,int depth){
	struct list_head *list;
	struct task_struct *task;
    tree.processes[tree.cnt].pid= p->pid;
    tree.processes[tree.cnt++].depth= depth;
	list_for_each(list, &(p->children)){
    	task = list_entry(list, struct task_struct, sibling);
		process_traverse_helper(task, depth+1);
	}

}
asmlinkage long sys_mycall(struct process_tree __user * buf)
{
    printk(KERN_ALERT "my_system_call\n");
    int depth = 0;
	struct task_struct *p = &init_task;
    tree.cnt=0;//清空计数器
	process_traverse_helper(p, depth);//从0号进程开始深度遍历
	// 将内核数据拷贝到用户态
	if(copy_to_user((struct process_tree *)buf, &tree, sizeof(struct process_tree ))){
		return -1;
	}
	return 0;
}

// 内核模块入口函数
static int __init syscall_init_module(void){
	sys_call_table = (unsigned long *)sys_call_table_address;
	saved =  sys_call_table[my_syscall_num];//保存该位置的原始数据
    make_rw((unsigned long)&sys_call_table[my_syscall_num]);//获取写权限           
	sys_call_table[my_syscall_num] = (unsigned long)&sys_mycall;//写入入口地址
    make_ro((unsigned long)&sys_call_table[my_syscall_num]); //恢复为只读                                     
	return 0;
}

//内核模块出口函数，恢复sys_call_table
static void __exit syscall_cleanup_module(void){
    make_rw((unsigned long)&sys_call_table[my_syscall_num]);//获取写权限           
	sys_call_table[my_syscall_num] = saved;//恢复原始数据
    make_ro((unsigned long)&sys_call_table[my_syscall_num]); //恢复为只读   
}

//导出该模块
module_init(syscall_init_module);
module_exit(syscall_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hook Module");  
