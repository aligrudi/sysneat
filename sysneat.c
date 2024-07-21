/*
 * SYSNEAT - A SMALL LINUX INIT PROGRAM
 *
 * Copyright (C) 2024 Ali Gholami Rudi <ali at rudi dot ir>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define M(s)	("\33[1;34m" s "\33[0m\n")

static char *rc[] = {"/etc/rc.up", NULL};
static char *rc_down[] = {"/etc/rc.down", NULL};
static char *rc_tty1[] = {"/sbin/getty", "38400", "tty1", "linux", NULL};
static char *rc_tty2[] = {"/sbin/getty", "38400", "tty2", "linux", NULL};

static int run(char *argv[], char *tty)
{
	int ret = fork();
	if (ret == 0) {
		if (tty != NULL) {
			setsid();
			close(0);
			close(1);
			close(2);
			open(tty, O_RDWR);
			dup(0);
			dup(0);
		}
		execvp(argv[0], argv);
		fprintf(stderr, M("sysneat: exec failed (%s)\n"), argv[0]);
		exit(1);
	}
	if (ret < 0)
		fprintf(stderr, M("sysneat: fork failed\n"));
	if (ret > 0)
		waitpid(ret, NULL, 0);
	return ret < 0;
}

static int run_repeat(char *argv[], char *tty)
{
	if (fork() == 0) {
		signal(SIGUSR1, SIG_DFL);
		signal(SIGUSR2, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
		while (run(argv, tty) >= 0)
			sleep(2);
	}
	return 0;
}

static void sigchld(int signo)
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}

static void sigboot(int signo)
{
	if (fork() != 0)
		return;
	printf(M("SYSNEAT: STOPPING..."));
	printf(M("+ sending SIGTERM..."));
	kill(-1, SIGTERM);
	sleep(5);
	printf(M("+ sending SIGKILL..."));
	kill(-1, SIGKILL);
	printf(M("+ running rc scripts..."));
	sync();
	run(rc_down, NULL);
	printf(M("+ halting..."));
	sleep(3);
	if (signo == SIGINT)
		reboot(RB_AUTOBOOT);
	if (signo == SIGUSR1 || signo == SIGUSR2)
		reboot(RB_POWER_OFF);
}

int main(void)
{
	printf(M("SYSNEAT: USERSPACE INIT"));
	signal(SIGUSR1, sigboot);
	signal(SIGUSR2, sigboot);
	signal(SIGINT, sigboot);
	signal(SIGCHLD, sigchld);

	printf(M("+ mounting base filesystem..."));
	mount("none", "/proc", "proc", 0, NULL);
	mount("dev", "/dev", "devtmpfs", MS_NOSUID | MS_NOATIME, NULL);
	mount("sys", "/sys", "sysfs", MS_NOSUID | MS_NOATIME, NULL);
	mount("tmp", "/tmp", "tmpfs", MS_NOSUID | MS_NOATIME, NULL);
	mkdir("/dev/pts", 0755);
	mount("pts", "/dev/pts", "devpts", MS_NOSUID | MS_NOATIME, NULL);

	printf(M("+ running init scripts..."));
	setenv("USER", "root", 0);
	setenv("HOME", "/", 0);
	run(rc, NULL);
	run_repeat(rc_tty1, "/dev/tty1");
	run_repeat(rc_tty2, "/dev/tty2");
	printf(M("+ done."));
	while (1)
		sleep(3600);
	return 0;
}
