/*
 * ���ļ���ҪΪptrace�ĺ����߼�����ptrace.h�º��������˷�װ
 */
#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#ifdef ANDROID
#include <linux/user.h>
#else
#include <sys/user.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <utils.h>

#include <stdarg.h>
#include "linker.h"

static regs_t oldregs;

dl_fl_t ldl;

/*
 * ��ʾ�Ĵ�����Ϣ
 */
void ptrace_dump_regs(regs_t *regs, char *msg) {
    int i = 0;
    printf("------regs %s-----\n", msg);
    for (i = 0; i < 18; i++) {
        printf("r[%02d]=%lx\n", i, regs->uregs[i]);
    }
}

/*
 * ����pid����ֹͣ����
 */
void ptrace_attach(int pid) {
    regs_t regs;
    int status = 0;
    /*
     * ��ʽ��ptrace(PTRACE_ATTACH,pid)
     * ����������ָ��pid ���̡�pid��ʾ�����ٽ��̡������ٽ��̽���Ϊ��ǰ���̵��ӽ��̣���������ֹ״̬��
     */
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL ) < 0) {
        perror("ptrace_attach");
        exit(-1);
    }
    /*�ж��ӽ���ֹͣ�Ƿ���SIGSTOP����*/
    status = ptrace_wait_for_signal(pid, SIGSTOP);
    printf("ptrace_wait_for_signal: %d %d\n", __LINE__, status);
    //waitpid(pid, NULL, WUNTRACED);

    ptrace_readreg(pid, &regs);
    memcpy(&oldregs, &regs, sizeof(regs));

    ptrace_dump_regs(&oldregs, "old regs");
#ifdef ANDROID
#ifdef THUMB
    regs.ARM_pc = 0x11;
    regs.ARM_cpsr |=0x30;
#else
    regs.ARM_pc= 0;
#endif
#else
    regs.rip = 0;
#endif
    ptrace_writereg(pid, &regs);

    ptrace_cont(pid);

    printf("waiting.. sigal...\n");

    /*
     * �쳣����,SIGSEGV�ǵ�һ������ִ����һ����Ч���ڴ�����,�����δ���ʱ���͸������ź�
     * ����ִ�к�ᷢ��signal 11 ��ע�����ʧȥ��Ӧ������
     */
    status = ptrace_wait_for_signal(pid, SIGSEGV);
    printf("ptrace_wait_for_signal2: %d %d\n", __LINE__, status);

}

/*
 * ��ʽ��ptrace(PTRACE_CONT, pid, 0, signal)
 * ����������ִ�С�pid��ʾ�����ٵ��ӽ��̣�signalΪ0�����������Խ�����ֹ���źţ�����Ϊ0����������ź�signal��
 */
void ptrace_cont(int pid) {
    //int stat;

    if (ptrace(PTRACE_CONT, pid, NULL, NULL ) < 0) {
        perror("ptrace_cont");
        exit(-1);
    }

    //while (!WIFSTOPPED(stat))
    //    waitpid(pid, &stat, WNOHANG);
}

void ptrace_detach(int pid) {
    ptrace_writereg(pid, &oldregs);

    if (ptrace(PTRACE_DETACH, pid, NULL, NULL ) < 0) {
        perror("ptrace_detach");
        exit(-1);
    }
}

/*
 * ��ʽ��ptrace(PTRACE_POKETEXT, pid, addr, data)
      ptrace(PTRACE_POKEDATA, pid, addr, data)
 * ���������ڴ��ַ��д��һ���ֽڡ�pid��ʾ�����ٵ��ӽ��̣��ڴ��ַ��addr������dataΪ��Ҫд������ݡ�
 */
void ptrace_write(int pid, unsigned long addr, void *vptr, int len) {
    int count;
    long word;
    void *src = (long*) vptr;
    count = 0;

    while (count < len) {
        memcpy(&word, src + count, sizeof(word));
        word = ptrace(PTRACE_POKETEXT, pid, (void*) (addr + count), (void*) word);
        count += 4;

        if (errno != 0)
            printf("ptrace_write failed\t %ld\n", addr + count);
    }
}

/*
 *��pid��addr��ʼ��ȡlen���ֽ�
 */
void ptrace_read(int pid, unsigned long addr, void *vptr, int len) {
    int i, count;
    long word;
    unsigned long *ptr = (unsigned long *) vptr;

    i = count = 0;

    /*
     * ��ʽ��ptrace(PTRACE_PEEKTEXT, pid, addr, data)
     * 	   ptrace(PTRACE_PEEKDATA, pid, addr, data)
     * ���������ڴ��ַ�ж�ȡһ���ֽڣ�pid��ʾ�����ٵ��ӽ��̣��ڴ��ַ��addr������dataΪ�û�������ַ���ڷ��ض��������ݡ�
     * ��Linux��i386�����û���������û����ݶ��غ����Զ�ȡ����κ����ݶ����ݴ�����һ���ġ�
     */
    while (count < len) {
        word = ptrace(PTRACE_PEEKTEXT, pid, (void*) (addr + count), NULL );
        count += 4;
        ptr[i++] = word;
    }
}

char * ptrace_readstr(int pid, unsigned long addr) {
    char *str = (char *) malloc(64);
    int i, count;
    long word;
    char *pa;

    i = count = 0;
    pa = (char *) &word;

    while (i <= 60) {
        word = ptrace(PTRACE_PEEKTEXT, pid, (void*) (addr + count), NULL );
        count += 4;

        if (pa[0] == '\0') {
            str[i] = '\0';
            break;
        } else
            str[i++] = pa[0];

        if (pa[1] == '\0') {
            str[i] = '\0';
            break;
        } else
            str[i++] = pa[1];

        if (pa[2] == '\0') {
            str[i] = '\0';
            break;
        } else
            str[i++] = pa[2];

        if (pa[3] == '\0') {
            str[i] = '\0';
            break;
        } else
            str[i++] = pa[3];
    }
    return str;
}

/*
 * ��ȡ�Ĵ�����ֵ����ӡ
 */
void ptrace_readreg(int pid, regs_t *regs) {
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs))
        printf("*** ptrace_readreg error ***\n");

}

void ptrace_writereg(int pid, regs_t *regs) {
    if (ptrace(PTRACE_SETREGS, pid, NULL, regs))
        printf("*** ptrace_writereg error ***\n");
}

/*
 * ����ѹջ
 */
unsigned long ptrace_push(int pid, regs_t *regs, void *paddr, int size) {
#ifdef ANDROID
    unsigned long arm_sp;
    arm_sp = regs->ARM_sp;
    arm_sp -= size;
    arm_sp = arm_sp - arm_sp % 4;
    regs->ARM_sp= arm_sp;

    //���ٿռ�д�����
    ptrace_write(pid, arm_sp, paddr, size);
    return arm_sp;
#else
    unsigned long esp;
    regs_t regs;
    ptrace_readreg(pid, &regs);
    esp = regs.esp;
    esp -= size;
    esp = esp - esp % 4;
    regs.esp = esp;
    ptrace_writereg(pid, &regs);
    ptrace_write(pid, esp, paddr, size);
    return esp;
#endif
}

long ptrace_stack_alloc(pid_t pid, regs_t *regs, int size) {
    unsigned long arm_sp;
    arm_sp = regs->ARM_sp;
    arm_sp -= size;
    arm_sp = arm_sp - arm_sp % 4;
    regs->ARM_sp= arm_sp;
    return arm_sp;
}

/*
 * ʹ�ӽ��̼����Զ����
 */
void *ptrace_dlopen(pid_t pid, const char *filename, int flag) {
#ifdef ANDROID
    regs_t regs;
    //int stat;
    ptrace_readreg(pid, &regs);

    ptrace_dump_regs(&regs, "before call to ptrace_dlopen\n");

#ifdef THUMB
    regs.ARM_lr = 1;
#else
    regs.ARM_lr= 0;
#endif

    //����dlopen������r0��/dev/libhook.so��ջ��ַ
    regs.ARM_r0= (long)ptrace_push(pid,&regs, (void*)filename,strlen(filename)+1);
    regs.ARM_r1= flag;
    regs.ARM_pc= ldl.l_dlopen;
    ptrace_writereg(pid, &regs);

    ptrace_dump_regs(&regs, "before continue ptrace_dlopen\n");
    ptrace_cont(pid);

    //��׽�쳣���ж��ӽ���
    printf("done %d\n", ptrace_wait_for_signal(pid, SIGSEGV));
    ptrace_readreg(pid, &regs);
    ptrace_dump_regs(&regs, "before return ptrace_dlopen\n");
    return (void*) regs.ARM_r0;
#endif
}

/*
 * �ڿ��в��ҷ��ţ�handle ָ���ֵΪ���ص�ַ
 */
void *ptrace_dlsym(pid_t pid, void *handle, const char *symbol) {

#ifdef ANDROID
    regs_t regs;
    //int stat;��ȡ�ӽ��̵ļĴ���ֵ
    ptrace_readreg(pid, &regs);
    ptrace_dump_regs(&regs, "before call to ptrace_dlsym\n");

#ifdef THUMB

    regs.ARM_lr = 1;
#else
    regs.ARM_lr= 0;
#endif

    regs.ARM_r0= (long)handle;
    regs.ARM_r1= (long)ptrace_push(pid,&regs, (void*)symbol,strlen(symbol)+1);

    regs.ARM_pc= ldl.l_dlsym;
    ptrace_writereg(pid, &regs);
    ptrace_dump_regs(&regs, "before continue ptrace_dlsym\n");
    ptrace_cont(pid);
    printf("done %d\n", ptrace_wait_for_signal(pid, SIGSEGV));
    ptrace_readreg(pid, &regs);
    ptrace_dump_regs(&regs, "before return ptrace_dlsym\n");

    //dlsym�ķ���ֵ��r0�У����鵽�ķ��ŵĵ�ַ
    return (void*) regs.ARM_r0;
#endif
}

int ptrace_mymath_add(pid_t pid, long mymath_add_addr, int a, int b) {
#ifdef ANDROID
    regs_t regs;
    //int stat;
    ptrace_readreg(pid, &regs);
    ptrace_dump_regs(&regs, "before call to ptrace_mymath_add\n");

#ifdef THUMB
    regs.ARM_lr = 1;
#else
    regs.ARM_lr= 0;
#endif

    regs.ARM_r0= a;
    regs.ARM_r1= b;

    regs.ARM_pc= mymath_add_addr;
    ptrace_writereg(pid, &regs);
    ptrace_cont(pid);
    printf("done %d\n", ptrace_wait_for_signal(pid, SIGSEGV));
    ptrace_readreg(pid, &regs);
    ptrace_dump_regs(&regs, "before return ptrace_mymath_add\n");

    return regs.ARM_r0;
#endif
}

int ptrace_call(int pid, long proc, int argc, ptrace_arg *argv) {
    int i = 0;
#define ARGS_MAX 64
    regs_t regs;
    ptrace_readreg(pid, &regs);
    ptrace_dump_regs(&regs, "before ptrace_call\n");

    /*prepare stacks*/
    for (i = 0; i < argc; i++) {
        ptrace_arg *arg = &argv[i];
        if (arg->type == PAT_STR) {
            arg->_stackid = ptrace_push(pid, &regs, arg->s, strlen(arg->s) + 1);
        } else if (arg->type == PAT_MEM) {
            //printf("push data %p to stack[%d] :%d \n", arg->mem.addr, stackcnt, *((int*)arg->mem.addr));
            arg->_stackid = ptrace_push(pid, &regs, arg->mem.addr, arg->mem.size);
        }
    }
    for (i = 0; (i < 4) && (i < argc); i++) {
        ptrace_arg *arg = &argv[i];
        if (arg->type == PAT_INT) {
            regs.uregs[i] = arg->i;
        } else if (arg->type == PAT_STR) {
            regs.uregs[i] = arg->_stackid;
        } else if (arg->type == PAT_MEM) {
            regs.uregs[i] = arg->_stackid;
        } else {
            printf("unkonwn arg type\n");
        }
    }

    for (i = argc - 1; i >= 4; i--) {
        ptrace_arg *arg = &argv[i];
        if (arg->type == PAT_INT) {
            ptrace_push(pid, &regs, &arg->i, sizeof(int));
        } else if (arg->type == PAT_STR) {
            ptrace_push(pid, &regs, &arg->_stackid, sizeof(unsigned long));
        } else if (arg->type == PAT_MEM) {
            ptrace_push(pid, &regs, &arg->_stackid, sizeof(unsigned long));
        } else {
            printf("unkonwn arg type\n");
        }
    }
#ifdef THUMB
    regs.ARM_lr = 1;
#else
    regs.ARM_lr= 0;
#endif
    regs.ARM_pc= proc;
    ptrace_writereg(pid, &regs);
    ptrace_cont(pid);
    printf("done %d\n", ptrace_wait_for_signal(pid, SIGSEGV));
    ptrace_readreg(pid, &regs);
    ptrace_dump_regs(&regs, "before return ptrace_call\n");

    //sync memory
    for (i = 0; i < argc; i++) {
        ptrace_arg *arg = &argv[i];
        if (arg->type == PAT_STR) {
        } else if (arg->type == PAT_MEM) {
            ptrace_read(pid, arg->_stackid, arg->mem.addr, arg->mem.size);
        }
    }

    return regs.ARM_r0;
}

/*
 * waitpid()�ķ�װ�����ڵȴ��ӽ��̷��أ������ؽ���״̬
 */
int ptrace_wait_for_signal(int pid, int signal) {
    int status;
    pid_t res;
    /*
     * waitpid()����ʱֹͣĿǰ���̵�ִ��,ֱ�����ź��������ӽ��̽����������ӽ��̽���״ֵ̬��
     * �ӽ��̵Ľ���״ֵ̬���ɲ��� status ����
     * WIFSTOPPED(status) ��Ϊ��ǰ��ͣ�ӽ��̷��ص�״̬����Ϊ�棻
     * �������������ִ��WSTOPSIG(status)��ȡʹ�ӽ�����ͣ���źű�š�
     */
    res = waitpid(pid, &status, 0);
    if (res != pid || !WIFSTOPPED (status))
        return 0;
    return WSTOPSIG (status) == signal;
}

/*
 * ��ȡ/system/bin/linker�Ŀ�ʼ��ַ�ͽ�����ַ
 */
static Elf32_Addr get_linker_base(int pid, Elf32_Addr *base_start, Elf32_Addr *base_end) {
    unsigned long base = 0;
    char mapname[FILENAME_MAX];
    memset(mapname, 0, FILENAME_MAX);

    /*�鿴���̵������ַ�ռ������ʹ�õ�*/
    snprintf(mapname, FILENAME_MAX, "/proc/%d/maps", pid);
    FILE *file = fopen(mapname, "r");
    *base_start = *base_end = 0;
    if (file) {
        //400a4000-400b9000 r-xp 00000000 103:00 139       /system/bin/linker
        while (1) {
            unsigned int atleast = 32;//��ƫ��������32
            int xpos = 20;
            char startbuf[9];
            char endbuf[9];
            char line[FILENAME_MAX];
            memset(line, 0, FILENAME_MAX);

            /*fgets ���ö�ȡһ��*/
            char *linestr = fgets(line, FILENAME_MAX, file);
            if (!linestr) {
                break;
            }
            printf("........%s <--\n", line);
            if (strlen(line) > atleast && strstr(line, "/system/bin/linker")) {
                memset(startbuf, 0, sizeof(startbuf));
                memset(endbuf, 0, sizeof(endbuf));

                memcpy(startbuf, line, 8);
                memcpy(endbuf, &line[8 + 1], 8);
                if (*base_start == 0) {
                    *base_start = strtoul(startbuf, NULL, 16);//�ַ���תΪ�޷�����
                    *base_end = strtoul(endbuf, NULL, 16);
                    base = *base_start;
                } else {
                    *base_end = strtoul(endbuf, NULL, 16);
                }
            }
        }
        fclose(file);

    }
    return base;

}

/*
 *��libdl.so�в���dlopen��dlclose��dlsym�ĺ���
 */
dl_fl_t *ptrace_find_dlinfo(int pid) {
    Elf32_Sym sym;
    Elf32_Addr addr;
    struct soinfo lsi;
#define LIBDLSO "libdl.so"
    Elf32_Addr base_start = 0;
    Elf32_Addr base_end = 0;

    /*linker ��Ҫ����ʵ�ֹ����ļ��������ӡ���֧��Ӧ�ó���Կ⺯������ʽ����ʽ���á�*/
    Elf32_Addr base = get_linker_base(pid, &base_start, &base_end);

    if (base == 0) {
        printf("no linker found\n");
        return NULL ;
    } else {
        printf("search libdl.so from %08u to %08u\n", base_start, base_end);
    }

    for (addr = base_start; addr < base_end; addr += 4) {
        char soname[strlen(LIBDLSO)];
        Elf32_Addr off = 0;

        //����/system/bin/linker�м��ص�libdl.so,����λ�ù̶�,������dlopen,dlcose,dlsym,dlerror
        ptrace_read(pid, addr, soname, strlen(LIBDLSO));
        if (strncmp(LIBDLSO, soname, strlen(LIBDLSO))) {
            continue;
        }

        //�ҵ�libdl.so�ļ���λ�ã�����ȡlibdl.so�Ķ�̬����Ϣ
        printf("soinfo found at %08u\n", addr);
        ptrace_read(pid, addr, &lsi, sizeof(lsi));
        printf("symtab: %p\n", lsi.symtab);

        //���ű�������һ�������ڶ�λ���ض�λʱ��Ҫ�Ķ�������õ���Ϣ��
        /*
         * �ڷ��ű�.symtab���У���Ҳ����α�Ľṹһ������һ�����飬ÿ������Ԫ����һ���̶��Ľṹ��������ŵ������Ϣ��
         * ����������������ַ��������Ǹ÷��������ַ�������±꣩�����Ŷ�Ӧ��ֵ�������Ƕ��е�ƫ�ƣ�Ҳ�����Ƿ��ŵ������ַ�������Ŵ�С���������͵Ĵ�С���ȵȡ�
         * ���ű��м�¼��һ����ȫ�ַ��ţ�����ȫ�ֱ�����ȫ�ֺ����ȵȡ�
         *
         * ÿ��objectҪ��ʹ����������ELF�ļ����ã���Ҫ�õ����ű�(symbol table)��
         * symbol entry.��ʵ�ϣ�һ��symbol entry �Ǹ�symbol�ṹ�������������
         * symbol�����ֺ͸�symbol��value.symbol name��������Ϊdynamic string
         * table������(index). The value of a symbol����ELF OBJECT�ļ��ڸ�
         * symbol�ĵ�ַ���õ�ַͨ����Ҫ�����¶�λ�����ϸ�objectװ�ص��ڴ�Ļ���ַ(base load address)��.
         */
        off = (Elf32_Addr)lsi.symtab;

        /*
         * ���˶γ�����Ѷ�����lsi.symtab��ʵָ�ľ���libdl.so���ص��ڴ��е� .dynsym
         * �ڲ鿴ģ����������libdl.so����ֻ�ж�̬���ű�
         * symtab �� .dynsym ���ŷ�����Ϣ(��Щ���Ű����ļ��������������������ȵ�)��ǰ��һ���Ǿ�̬���ţ��������Ƕ�̬������ط��ţ�
         *
         * sh_flag ��Ա������ʾ��������ر�־��ȡ��ͬ��ֵ�в�ͬ�����壬������Ա�ʾ�ý����ǲ��Ǵ�ſ�ִ�д��룬�ý����Ƿ�������ڽ���ִ��ʱ��д�����ݵȡ�
         * ������һ�� A ��־���� Allocable ֮�⣬��ʾ�ڳ�������ʱ��������Ҫʹ�����ǣ��������ǻᱻ���ص��ڴ���ȥ������ .data һ����� allocable �ġ� ��֮������ non-Allocable�������͵Ľ���ֻ�Ǳ��������������������������ƹ�����ʹ�ã���������������ʱ���ڴ���ȥ���� .symtab �� .strtab �Լ����� .debug ��ؽ����������������͡��ڿ�ִ���ļ�ִ��ʱ��allocable ���ֻᱻ���ص��ڴ��У��� non-Allocable �������������ļ��ڡ�
         *
         */
        ptrace_read(pid, off, &sym, sizeof(sym));
        //just skip
        off += sizeof(sym);

        /*
         * ����˳������⣬��ʾ��ӡ.dynsym�ڣ���android2.3.3����˳����ʵ��������
         * Symbol table '.dynsym' contains 28 entries:
   	   	   Num:    Value  Size Type    Bind   Vis      Ndx Name
     	 	 0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND
     	 	 1: 00001a0d     4 FUNC    GLOBAL DEFAULT    7 dlopen
     	 	 2: 00001a11     4 FUNC    GLOBAL DEFAULT    7 dlerror
     	 	 3: 00001a15     4 FUNC    GLOBAL DEFAULT    7 dlsym
     	 	 4: 00001a19     4 FUNC    GLOBAL DEFAULT    7 dladdr
     	 	 5: 00001a1d     4 FUNC    GLOBAL DEFAULT    7 dlclose
     	 * ����dlcloseû���õ������Ըó������λ�û��ǶԵġ�
     	 */

        ptrace_read(pid, off, &sym, sizeof(sym));
        printf("name2:%d\n",sym.st_name);
        printf("value2:%d\n",sym.st_value);
        ldl.l_dlopen = sym.st_value;
        off += sizeof(sym);

        ptrace_read(pid, off, &sym, sizeof(sym));
        printf("name3:%d\n",sym.st_name);
        printf("value3:%d\n",sym.st_value);
        ldl.l_dlclose = sym.st_value;
        off += sizeof(sym);

        ptrace_read(pid, off, &sym, sizeof(sym));
        printf("name4:%d\n",sym.st_name);
        printf("value4:%d\n",sym.st_value);
        ldl.l_dlsym = sym.st_value;
        off += sizeof(sym);

        printf("dlopen addr %p\n", (void*) ldl.l_dlopen);
        printf("dlclose addr %p\n", (void*) ldl.l_dlclose);
        printf("dlsym addr %p\n", (void*) ldl.l_dlsym);
        return &ldl;

    }
    printf("%s not found!\n", LIBDLSO);
    return NULL ;
}

/*
 * ���ݽ������Ʋ��ҽ��̵�pid
 */
int find_pid_of( const char *process_name )
{
	int id;
	pid_t pid = -1;
	DIR* dir;
	FILE *fp;
	char filename[32];
	char cmdline[256];

	/*Ϊ�˻�ȡĳ�ļ���Ŀ¼���ݣ���ʹ�õĽṹ�塣*/
	struct dirent * entry;

	if ( process_name == NULL )
		return -1;

	dir = opendir( "/proc" );
	if ( dir == NULL )
		return -1;

	while( (entry = readdir( dir )) != NULL )
	{
		id = atoi( entry->d_name );
		if ( id != 0 )
		{
			/*/proc/pid/cmdline�����������ʱִ�е����һ��Ϊ��������*/
			sprintf( filename, "/proc/%d/cmdline", id );
			fp = fopen( filename, "r" );
			if ( fp )
			{
				fgets( cmdline, sizeof(cmdline), fp );
				fclose( fp );

				if ( strcmp( process_name, cmdline ) == 0 )
				{
					/* process found */
					pid = id;
					break;
				}
			}
		}
	}

	closedir( dir );

	return pid;
}

