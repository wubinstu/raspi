#include "file.h"
#include "msg.h"

long file_size(const char * file_path)
{
    struct stat statbuf;
    int state = stat (file_path,&statbuf);
    if(state == -1)
    {
    	perr(true,false,"function stat returns -1 at file_size");
    	return -1;
    }
    return statbuf.st_size;
}

int readopen(const char * file_path)
{
	if(access(file_path,F_OK)  == -1)
	{
		perr(true,true,"readopen: file does not exist");
		return -1;
	}
    int fd = open(file_path,O_RDONLY);
    return fd;
}

int writeopen(const char * file_path)
{
    int fd;
    if(access(file_path,F_OK) == -1)
    {
        fd = creat(file_path,FILE_MODE);
        perr(fd == -1,true,"writeopen: file does not exist and cannot be created");
        close(fd);
    }
    fd = open(file_path,O_WRONLY);
    return fd;
}

int rwopen(const char * file_path)
{
    int fd;
    if(access(file_path,F_OK) == -1)
    {
        fd = creat(file_path,FILE_MODE);
        perr(fd == -1,true,"rwopen: file does not exist and cannot be created");
        close(fd);
    }
    fd = open(file_path,O_RDONLY|O_WRONLY);
    return fd;
}

int newopen(const char * file_path)
{
    int fd;
    if(access(file_path,F_OK) == -1)
    {
        fd = creat(file_path,FILE_MODE);
        perr(fd == -1,true,"newopen: file does not exist and cannot be created");
        close(fd);
    }
    fd = open(file_path,O_RDONLY|O_WRONLY|O_TRUNC);
    return fd;
}

long fcopyfile(int source_fd,int destination_fd)
{
    bool warning_level = false;
    // true for error,false for warning
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
            perr(true,warning_level,"function read returns -1 at fcopyfile");
            return count_size;
        }
        count_size += read_size;
        write_size = write(destination_fd,buf,read_size);
        if(write_size != read_size)
        {
            perr(true,warning_level,"the size of read and write is unequal at fcopyfile");
            return count_size;
        }
    } while (read_size > 0);
    return count_size;
}