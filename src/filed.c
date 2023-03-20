#include "filed.h"
#include "log.h"
#include "global.h"


void setFiledLogLevel (int logLevel)
{
    if (logLevel >= 0 && logLevel <= 7)
        filed_logLevel = logLevel;
}

long fileSize (const char *file_path)
{
    int errno_save = errno;
    struct stat stat_buf;
    int state = stat (file_path, &stat_buf);
    if (state == -1)
    {
        perr (true, filed_logLevel,
              "function stat returns %d when you called fileSize", state);
        return -1;
    }
    errno = errno_save;
    return stat_buf.st_size;
}

int readOpen (const char *file_path)
{
    int errno_save = errno;
    int state = access (file_path, F_OK);
    if (state == -1)
    {
        perr (state == -1, filed_logLevel,
              "function access(F_OK) returns %d when you called readOpen", state);
        return state;
    }
    int fd = open (file_path, O_RDONLY);
    perr (fd == -1, filed_logLevel,
          "function open returns %d when you called readOpen", fd);
    errno = errno_save;
    return fd;
}

int writeOpen (const char *file_path)
{
    int errno_save = errno;
    int fd;
    if (access (file_path, F_OK) == -1)
    {
        fd = creat (file_path, FILE_MODE);
        if (fd == -1)
        {
            perr (fd == -1, filed_logLevel,
                  "function access(F_OK) returns -1 and "
                  "function create returns %d when you called writeOpen", fd);
            close (fd);
            return fd;
        }
    }
    fd = open (file_path, O_WRONLY);
    perr (fd == -1, filed_logLevel,
          "function open returns %d when you called writeOpen", fd);
    errno = errno_save;
    return fd;
}

int rwOpen (const char *file_path)
{
    int errno_save = errno;
    int fd;
    if (access (file_path, F_OK) == -1)
    {
        fd = creat (file_path, FILE_MODE);
        if (fd == -1)
        {
            perr (fd == -1, filed_logLevel,
                  "function access(F_OK) returns -1 and "
                  "function create returns %d when you called rwOpen", fd);
            close (fd);
            return fd;
        }
    }
    fd = open (file_path, O_RDWR);
    perr (fd == -1, filed_logLevel,
          "function open returns %d when you called rwOpen", fd);
    errno = errno_save;
    return fd;
}

int newOpen (const char *file_path)
{
    int errno_save = errno;
    int fd;
    if (access (file_path, F_OK) == -1)
    {
        fd = creat (file_path, FILE_MODE);
        if (fd == -1)
        {
            perr (true, filed_logLevel,
                  "function access(F_OK) returns -1 and "
                  "function create returns %d when you called newOpen", fd);
            close (fd);
            return fd;
        }
    }
    fd = open (file_path, O_RDWR | O_TRUNC);
    perr (fd == -1, filed_logLevel,
          "function open returns %d when you called newOpen", fd);
    errno = errno_save;
    return fd;
}

long copyFile (int source_fd, int destination_fd)
{
    int errno_save = errno;
    char buf[BUF_SIZE];
    long read_size, write_size, count_size = 0;
    lseek (source_fd, 0, SEEK_SET);
    lseek (destination_fd, 0, SEEK_SET);

    do
    {
        read_size = read (source_fd, buf, BUF_SIZE);
        if (read_size == -1)
        {
            //read error,stop
            perr (true, filed_logLevel,
                  "function read returns -1 when you called copyFile");
            return count_size;
        }
        count_size += read_size;
        write_size = write (destination_fd, buf, read_size);
        if (write_size != read_size)
        {
            perr (true, filed_logLevel,
                  "the size of read and write is unequal "
                  "when you called copyFile");
            return count_size;
        }
    } while (read_size > 0);
    errno = errno_save;
    return count_size;
}