/*
 * process.h
 *
 *  Created on: 2013-8-29
 *      Author: Administrator
 */

#ifndef PROCESS_H_
#define PROCESS_H_
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <asm/ptrace.h>
#include <asm/user.h>
#include <asm/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <jni.h>
#include <elf.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "utils.h"
#include<errno.h>
#include <signal.h>
#include <sys/types.h>
#ifdef ANDROID
//#include <linker.h>
#endif
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <jni.h>

#define FIFO "/data/tmp/fifo"

int hook_entry();
int pipe_write(const char *key);
char* pipe_read(char buf[]);

#endif /* PROCESS_H_ */
