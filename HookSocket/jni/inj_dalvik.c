/*
 * ����ļ���������ע����ںͷ��ű��滻�߼���
 * 1���ڸú����м���libhook.soͨ�����е�do_hook��������ԭ����open��close��ַ�Լ�Ҫ�滻���µ�open��close������ַ
 * 2��Ȼ��̬��libnativehelper��̬�⣬��ȡ��ṹ�����ڱ����ҵ�ȫ�ַ��ű���GOT�������ñ��洢���ⲿ�������ŵĵ�ַ
 * 3������GOT���ҵ�ԭ�ȵ�open������close������ַ���ֱ��滻Ϊ�µ�open�������µ�close��������
 */

#include "process.h"
/*
 * �˴�����Ҫ���滻���ű���so�б�
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
	/*�˴�����Ҫע��Ľ���*/
	//char* process = "com.speedsoftware.rootexplorer";
	//char *process="in.wptraffcianalyzer.filereadwritedemo";

	pid = find_pid_of(process);
	ptrace_attach(pid);
	ptrace_find_dlinfo(pid);

	handle = ptrace_dlopen(pid, "/dev/libhook.so", 2);
	printf("ptrace_dlopen handle %p\n", handle);
	//hook_entry();

	/*�����滻open�����ķ��Ž�*/
	proc = (long) ptrace_dlsym(pid, handle, "new_sendto");
	printf("sendto = %lx\n", proc);
	LOGD("sendto = %lx\n", proc);
	replace_all_rels(pid, "sendto", proc, sos);

	/*�����滻open�����ķ��Ž�*/
	proc = (long) ptrace_dlsym(pid, handle, "new_recvfrom");
	printf("recvfrom = %lx\n", proc);
	LOGD("recvfrom = %lx\n", proc);
	replace_all_rels(pid, "recvfrom", proc, sos);

	ptrace_detach(pid);
}
