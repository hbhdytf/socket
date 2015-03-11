/*
 * process.c
 *
 *  Created on: 2013-8-29
 *      Author: Administrator
 */
#include "process.h"

/*
 *此处打开动态链接库，通过do_hook函数得到新旧地址
 */
int hook_entry() {
	unsigned long old_Posix_sendtoBytes_addr;
	unsigned long new_Posix_sendtoBytes_addr;
	unsigned long old_close_addr;
	unsigned long new_close_addr;
	LOGD("hello ARM! pid:%d\n", getpid());
	void *handle;
	/**
	 * 调用do_hook 函数
	 */
	int (*fcn)(unsigned long *param, unsigned long *param1,
			unsigned long *param2, unsigned long *param3);
	int target_pid = getpid();

	handle = dlopen("/dev/libhook.so", RTLD_NOW);
	LOGD("The Handle of libhook: %x\n", handle);

	if (handle == NULL) {
		LOGD("Failed to load libhook.so: %s\n", dlerror());
		return 1;
	}

	/* 动态打开do_hook函数*/
	LOGD("find do_hook pre %x\n", fcn);
	fcn = dlsym(handle, "do_hook");
	if (fcn != NULL)
		LOGD("find do_hook %x\n", fcn);
	else {
		LOGD("failed to find do_hook\n");
		return 0;
	}

	fcn(&old_Posix_sendtoBytes_addr, &new_Posix_sendtoBytes_addr, &old_close_addr, &new_close_addr);
	//取old_open_addr地址
	LOGD("[+] Get old address global  %x\n", old_Posix_sendtoBytes_addr);
	//取new_open_addr地址
	LOGD("[+] Get new address global  %x\n", new_Posix_sendtoBytes_addr);
	LOGD("[+] Get old address global  %x\n", old_close_addr);
	LOGD("[+] Get new address global  %x\n", new_close_addr);
	return 0;
}

int pipe_write(const char *key) {
	int fd;
	char w_buf[1024];
	int nwrite;
	LOGD("pid:%d\n",getpid());
	LOGD("call pipe write,%s\n", key);
	mkdir("/data/tmp", 0777);

	if (((fd=mkfifo(FIFO, O_CREAT | O_EXCL)) < 0) && (errno != EEXIST)) {
		LOGD("cannot create fifoserver\n");
		return -1;
	}
	LOGD("here fd:%d",fd);
	if (fd == -1)
		if (errno == ENXIO) {
			LOGD("open error;no reading process\n");
			return -1;
		}
	fd = open(FIFO, O_WRONLY|O_CREAT|O_TRUNC, 0);
	LOGD("fd:%d\n", fd);
	if (key == NULL) {
		LOGD("Please send something\n");
		return -1;
	}
	strcpy(w_buf, key);
	if ((nwrite = write(fd, w_buf, 1024)) == -1) {
		if (errno == EAGAIN)
			LOGD("The FIFO has not been read yet. Please try later\n");
	} else
		LOGD("write %s to the FIFO\n", w_buf);
	unlink(FIFO);
	return 1;
}

char* pipe_read(char buf[]) {
	char buf_r[1024];
	int fd;
	int nread;
	if (access(FIFO, F_OK) == -1) {
		LOGD("access error!\n");
		fd = mkfifo(FIFO, 0777);
	}
	LOGD("pid:%d\n",getpid());
	LOGD("Preparing for reading bytes....\n");
	memset(buf_r, 0, sizeof(buf_r));
	fd = open(FIFO, O_RDONLY, 0);

	LOGD("fd is %d\n",fd);
	int md = lseek(fd, 0L, SEEK_SET);
	LOGD("md_%d/t", md);
	memset(buf_r, 0, sizeof(buf_r));
	if ((nread = read(fd, buf_r, 1024)) == -1) {
		if (errno == EAGAIN)
			printf("no data yet\n");
	}
	LOGD("read %s from FIFO\n", buf_r);
	unlink(FIFO);
	strcpy(buf, buf_r);
	return buf;
}
