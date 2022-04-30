//
// Created by Einc on 2022/4/1.
//

#include "myprocess.h"
#include "raspi.h"
#include "msgd.h"
#include "myssl.h"
#include "conf.h"
#include "mysockd.h"

static int processLogLevel = LOG_WARNING;


extern confOpt co;
extern int serv_fd;
extern SSL * ssl_serv_fd;
extern SSL_CTX * ctx;
extern bool mode_strict;
extern bool mode_daemon;

extern jmp_buf myjmp;

//extern pthread_t thread_sender;
extern pthread_t thread_cheker;


void
daemonize (const char *cmd)
{
	errno = 0;
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
	errno = 0;
	printf ("\n");
	perr_d (true,LOG_INFO,"Service Will Exit After Cleanup");
	
	if(thread_cheker != 0)
		pthread_cancel (thread_cheker);
	
	if(check_fd (serv_fd))
	{
		if(mode_strict)
		{
			SSL_write (ssl_serv_fd,"FIN",4);
			SSL_shutdown (ssl_serv_fd);
			SSL_free (ssl_serv_fd);
			SSL_CTX_free (ctx);
		}
		else write (serv_fd,"FIN",4);
		close (serv_fd);
	}
	turn_off_led (LED_YEL);
	turn_off_led (LED_RED);
	turn_off_led (LED_GRE);
	if(access (PID_FILE,F_OK) == 0)
		unlink (PID_FILE);
	fflush (NULL);
	exit (0);
}

void reset()
{
	turn_off_led (LED_YEL);
	turn_off_led (LED_RED);
	turn_off_led (LED_GRE);
	fflush (NULL);
	
	if(check_fd (serv_fd))
		close (serv_fd);
	
	default_confOpt (&co);
	
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
	errno = 0;
	if (access (PID_FILE,F_OK) == 0)
	{
		perr_d (true,LOG_ERR,"service already running! stopped. (If Not,Please type \"--clean\" to remove %s)",PID_FILE);
		my_exit();
	}
	int fd = open (PID_FILE,O_CREAT|O_RDWR|O_TRUNC,FILE_MODE);
	if(fd < 0)
	{
		perr_d (true,LOG_ERR,"PID file can NOT be created");
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

bool check_permission(const char * msg)
{
	errno = 0;
	bool flag = (geteuid() == 0);
	perr_d (!flag,LOG_NOTICE,"root permission are required: %s",msg);
	return flag;
}

void sig_reg()
{
	errno = 0;
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

void dealWithArgs(int argc,const char * argv[])
{
	errno = 0;
	for(int i = 1;i < argc;i++)
	{
		if(strcmp (argv[i],"--daemon") == 0)
		{
			mode_daemon = true;
			continue;
		}
		else if(strcmp (argv[i],"--strict") == 0)
		{
			mode_strict = true;
			continue;
		}
		else if(strcmp (argv[i],"--clean") == 0)
		{
			if(!check_permission ("to delete PID_FILE"))
				exit (-1);
			printf ("%s will be deleted\n",PID_FILE);
			unlink (PID_FILE);
			exit (0);
		}
		else if(strcmp (argv[i],"--default-conf") == 0)
		{
			printf ("reset %s\n",CONF_FILE);
			unlink (CONF_FILE);
			defaultconf (CONF_FILE);
			exit (0);
		}
		else if(strcmp (argv[i],"--settings") == 0)
		{
			if(!check_permission ("or it will open in read-only mode(Press Any Key To Continue"))
				getchar();
			execlp ("vim","vim",CONF_FILE,NULL);
		}
		
		else if(strcmp (argv[i],"--help") == 0)
		{
			printf ("Usage:  [--daemon] \t Runing in daemon mode\n\t[--strict] \t Entering strict mode\n\t"
					"[--clean] \t Delete PID file\n\t[--default-conf] Create(Overwrite) default configuration file\n\t[--settings] \t Open(VIM editor) configuration file\n");
			exit (0);
		}
		else
		{
			printf ("Unknown Parameter, Use \"--help\" for help information\n");
			exit (-1);
		}
	}
	printf ("daemonMode = %s,strictMode = %s\n",mode_daemon?"true":"false",mode_strict?"true":"false");
}