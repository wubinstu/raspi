
#ifndef __FILED_H_
#define __FILED_H_


#include "head.h"

extern void setFiledLogLevel(int logLevel);

/** return file size */
extern long file_size_d(const char * file_path);

/**
 * Open the file in read-only mode.
 * If the file does not exist, it will not be created */
extern int readopen_d(const char * file_path);

/**
 * Open the file in write only mode.
 * If the file does not exist, create the file */
extern int writeopen_d(const char * file_path);

/**
 * Open the file in read-write mode.
 * If the file does not exist, create the file */
extern int rwopen_d(const char * file_path);

/** Overwrite opens a completely new file regardless of whether the file exists or not */
extern int newopen_d(const char * file_path);

/** Copy files completely from scratch */
extern long fcopyfile_d(int source_fd,int destination_fd);

#endif