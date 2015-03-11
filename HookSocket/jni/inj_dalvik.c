/*
 * 这个文件编译生成注入入口和符号表替换逻辑。
 * 1、在该函数中加载libhook.so通过其中的do_hook函数返回原来的open和close地址以及要替换的新的open和close函数地址
 * 2、然后静态打开libnativehelper动态库，读取其结构遍历节表，找到全局符号表（GOT表），该表存储了外部依赖符号的地址
 * 3、遍历GOT表找到原先的open函数和close函数地址，分别替换为新的open函数和新的close函数即可
 */

#include "process.h"
/*
 * 此处定义要被替换符号表的so列表
 */
//char *sos[] = { "libnativehelper.so","libjavacore.so","libc.so" };
char *sos[] = { "libjavacore.so" };
int main(int argc, char *argv[]) {
	int pid = 0;

	void *handle = NULL;
	long proc = 0;

	char *process = argv[1];
	LOGD("argc:%d\n", argc);
	LOGD("process2:%s\n", process);
	/*此处定义要注入的进程*/
	//char* process = "com.speedsoftware.rootexplorer";
	//char *process="in.wptraffcianalyzer.filereadwritedemo";

	pid = find_pid_of(process);
	ptrace_attach(pid);
	ptrace_find_dlinfo(pid);

	handle = ptrace_dlopen(pid, "/dev/libhook.so", 2);
	printf("ptrace_dlopen handle %p\n", handle);
	//hook_entry();

	/*查找替换open函数的符号节*/
	proc = (long) ptrace_dlsym(pid, handle, "new_sendto");
	printf("sendto = %lx\n", proc);
	LOGD("sendto = %lx\n", proc);
	replace_all_rels(pid, "sendto", proc, sos);

	/*查找替换open函数的符号节*/
	proc = (long) ptrace_dlsym(pid, handle, "new_recvfrom");
	printf("recvfrom = %lx\n", proc);
	LOGD("recvfrom = %lx\n", proc);
	replace_all_rels(pid, "recvfrom", proc, sos);

	ptrace_detach(pid);
}

