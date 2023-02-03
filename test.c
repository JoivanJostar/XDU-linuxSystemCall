#include <linux/unistd.h>
#include <syscall.h>
#include <sys/types.h>
#include <stdio.h>

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

struct process_tree tree; //进程树

int main(void){
	int ret_code=syscall(300, &tree);
    printf("%d\n",ret_code);
	for(int i = 0; i < tree.cnt; i++){
		for(int j = 0; j < tree.processes[i].depth; j++){
			printf("  ");
		}
		printf("|--%d\n", tree.processes[i].pid);                            	
	}
	return 0;
}
