#include "filed.h"
#include "msgd.h"

// Default LOG_LEVEL : LOG_WARNING
static int filed_logLevel = 4;

void setFiledLogLevel(int logLevel)
{
    filed_logLevel = logLevel;
}

long file_size_d(const char * file_path)
{
    struct stat statbuf;
    int state = stat (file_path,&statbuf);
    if(state == -1)
    {
    	perr_d(true,filed_logLevel,"function stat returns %d when you called file_size",state);
    	return -1;
    }
    return statbuf.st_size;
}

int readopen_d(const char * file_path)
{
    int state = access(file_path,F_OK);
    if(state == -1)
    {
	    perr_d (state == -1, filed_logLevel, "function access(F_OK) returns %d when you called readopen", state);
	    return state;
    }
    int fd = open(file_path,O_RDONLY);
	perr_d(fd == -1,filed_logLevel,"fucntion open returns %d when you called readopen",fd);
    return fd;
}

int writeopen_d(const char * file_path)
{
    int fd;
    if(access(file_path,F_OK) == -1)
    {
        fd = creat(file_path,FILE_MODE);
        if(fd == -1)
        {
	        perr_d (fd == -1, filed_logLevel,
	                "function access(F_OK) returns -1 and function create returns %d when you called writeopen", fd);
	        close (fd);
	        return fd;
        }
    }
    fd = open(file_path,O_WRONLY);
    perr_d(fd == -1,filed_logLevel,"function open returns %d when you called writeopen",fd);
    return fd;
}

int rwopen_d(const char * file_path)
{
    int fd;
    if(access(file_path,F_OK) == -1)
    {
        fd = creat(file_path,FILE_MODE);
        if(fd == -1)
        {
	        perr_d (fd == -1, filed_logLevel,
	                "function access(F_OK) returns -1 and function create returns %d when you called rwopen", fd);
	        close (fd);
	        return fd;
        }
    }
    fd = open(file_path,O_RDWR);
    perr_d(fd == -1,filed_logLevel,"function open returns %d when you called rwopen",fd);
    return fd;
}

int newopen_d(const char * file_path)
{
    int fd;
    if(access(file_path,F_OK) == -1)
    {
        fd = creat(file_path,FILE_MODE);
        if(fd == -1)
        {
	        perr_d (true, filed_logLevel,
	                "function access(F_OK) returns -1 and function create returns %d when you called newopen", fd);
	        close (fd);
	        return fd;
        }
    }
    fd = open(file_path,O_RDWR|O_TRUNC);
    perr_d(fd == -1,filed_logLevel,"function open returns %d when you called newopen",fd);
    return fd;
}

long fcopyfile_d(int source_fd,int destination_fd)
{
    char buf[BUF_SIZE];
    long read_size,write_size,count_size = 0;
    lseek (source_fd,0,SEEK_SET);
    lseek (destination_fd,0,SEEK_SET);

    do
    {
        read_size = read(source_fd,buf,BUF_SIZE);
        if(read_size == -1)
        {
            //read error,stop
            perr_d(true,filed_logLevel,"function read returns -1 when you called fcopyfile");
            return count_size;
        }
        count_size += read_size;
        write_size = write(destination_fd,buf,read_size);
        if(write_size != read_size)
        {
            perr_d(true,filed_logLevel,"the size of read and write is unequal when you called fcopyfile");
            return count_size;
        }
    } while (read_size > 0);
    return count_size;
}