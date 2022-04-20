
#ifndef __FILE_H_
#define __FILE_H_

#include "head.h"

/** return file size */
extern long file_size(const char * file_path);

/**
 * Open the file in read-only mode.
 * If the file does not exist, it will not be created */
extern int readopen(const char * file_path);

/**
 * Open the file in write only mode.
 * If the file does not exist, create the file */
extern int writeopen(const char * file_path);

/**
 * Open the file in read-write mode.
 * If the file does not exist, create the file */
extern int rwopen(const char * file_path);

/** Overwrite opens a completely new file regardless of whether the file exists or not */
extern int newopen(const char * file_path);

/** Copy files completely from scratch */
extern long fcopyfile(int source_fd,int destination_fd);

#endif