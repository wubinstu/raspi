//
// Created by Einc on 2022/4/1.
//

#include "myprocess.h"
#include "raspi.h"
#include "msgd.h"
#include "myssl.h"

static int processLogLevel = LOG_WARNING;

extern unsigned long serv_ip;
extern unsigned short serv_port;
extern int interval;
extern int frectime;
extern int frecatps;
extern int serv_fd;
extern char CAfile[256];
extern char UCert[256];
extern char UKey[256];
extern SSL * ssl_serv_fd;
extern SSL_CTX * ctx;
extern bool mode_strict;

extern jmp_buf myjmp;

//extern pthread_t thread_sender;
extern pthread_t thread_cheker;


void
daemonize (const char *cmd)
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;
	
	/**
     * Clear file creation mask
    */
	umask (0);
	
	/**
     * Get maximum number of file descriptors
    */
	if (getrlimit (RLIMIT_NOFILE, &rl) < 0)
		perr_d (true, processLogLevel, "%s: can not get file limit", cmd);
	
	/**
     * Become a session leader to lose controlling TTY
    */
	if ((pid = fork ()) < 0)
		perr_d (true, processLogLevel, "%s: cant not fork", cmd);
	else if (pid != 0)
		exit (0);
	
	setsid ();
	
	/**
     * Ensure future opens won't allocate controlling TTYs.
    */
	sa.sa_handler = SIG_IGN;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction (SIGHUP, &sa, NULL) < 0)
		perr_d (true, processLogLevel, "%s: can not ignore SIGHUP", cmd);
	
	if ((pid = fork ()) < 0)
		perr_d (true, processLogLevel, "%s: can not fork", cmd);
	else if (pid != 0)
		exit (0);
	
	/**
     * Change the current working directory to the root so
     * we won't prevent file system from being unmounted
    */
	if (chdir ("/") < 0)
		perr_d (true, processLogLevel, "%s: can not change directory to /", cmd);
	
	/**
     * close all open file descriptors
    */
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	
	for (i = 0; i < rl.rlim_max; i++)
		close (i);
	
	/**
     * Attach file descriptors 0,1 and 2 to /dev/null
    */
	fd0 = open ("/dev/null", O_RDWR);
	fd1 = dup (0);
	fd2 = dup (0);
	
	/**
     * Initialize the log file
    */
	openlog (cmd, LOG_CONS | LOG_PID, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2)
	{
		syslog (LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
		exit (1);
	}
}

void my_exit()
{
	pthread_cancel (thread_cheker);
//	pthread_cancel (thread_sender);
	if(mode_strict)
	{
		SSL_write (ssl_serv_fd,"FIN",4);
		SSL_shutdown (ssl_serv_fd);
		SSL_free (ssl_serv_fd);
		SSL_CTX_free (ctx);
	}
	else write (serv_fd,"FIN",4);
	turn_off_led (LED_YEL);
	turn_off_led (LED_RED);
	turn_off_led (LED_GRE);
	unlink (PID_FILE);
	fflush (NULL);
	close (serv_fd);
	exit (0);
}

void reset()
{
	turn_off_led (LED_YEL);
	turn_off_led (LED_RED);
	turn_off_led (LED_GRE);
	fflush (NULL);
	close (serv_fd);
	
	serv_ip = 0;
	serv_port = 0;
	interval = 10;
	frectime = 42;
	frecatps = 7;
	
	bzero (CAfile,sizeof(CAfile));
	bzero (UCert,sizeof(UCert));
	bzero (UKey,sizeof(UKey));
	
	longjmp (myjmp,RESET);
}

void sighup_handler()
{
	perr_d (true,LOG_INFO,"configure file reload, reset.");
	reset();
}

void sigalrm_handler()
{
	perr_d (true,LOG_WARNING,"Server response timeout, reset.");
	reset();
}

void check_running()
{
	if (access (PID_FILE,F_OK) == 0)
	{
		perr_d (true,LOG_ERR,"service already running! stopped.");
		my_exit();
	}
	int fd = open (PID_FILE,O_CREAT|O_RDWR|O_TRUNC,FILE_MODE);
	if(fd < 0)
	{
		perr_d (true,LOG_ERR,"PID file can NOT create");
		my_exit();
	}
	
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	fcntl(fd, F_SETLK, &fl);
	
	char pid[16] = {0};
	sprintf (pid,"%ld",(long)getpid());
	write (fd,pid, strlen (pid));
}

void sig_reg()
{
	struct sigaction newact,oldact;
	sigset_t set;
	sigfillset (&set);
	memset (&newact,0,sizeof(struct sigaction));
	memset (&oldact,0,sizeof(struct sigaction));
	
	newact.sa_handler = my_exit;
	newact.sa_mask = set;
	sigaction(SIGINT,&newact,&oldact);
	sigaction(SIGTERM,&newact,&oldact);
	
	
	memset (&newact,0,sizeof(struct sigaction));
	memset (&oldact,0,sizeof(struct sigaction));
	
	sigemptyset (&set);
//	sigaddset (&set,SIGHUP);
//	sigaddset (&set,SIGINT);
//	sigaddset (&set,SIGTERM);
	newact.sa_mask = set;
	newact.sa_handler = sighup_handler;
	sigaction(SIGHUP,&newact,&oldact);
	
	memset (&newact,0,sizeof(struct sigaction));
	memset (&oldact,0,sizeof(struct sigaction));
	
	sigemptyset (&set);
//	sigaddset (&set,SIGHUP);
//	sigaddset (&set,SIGALRM);
	newact.sa_mask = set;
	newact.sa_handler = sigalrm_handler;
	sigaction(SIGALRM,&newact,&oldact);
}