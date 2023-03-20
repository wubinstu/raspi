
#ifndef __FILED_H_
#define __FILED_H_


#include "head.h"

extern void setFiledLogLevel (int logLevel);

/** return file size */
extern long fileSize (const char *file_path);

/**
 * Open the file in read-only mode.
 * If the file does not exist, it will not be created */
extern int readOpen (const char *file_path);

/**
 * Open the file in write only mode.
 * If the file does not exist, create the file */
extern int writeOpen (const char *file_path);

/**
 * Open the file in read-write mode.
 * If the file does not exist, create the file */
extern int rwOpen (const char *file_path);

/** Overwrite opens a completely new file regardless of whether the file exists or not */
extern int newOpen (const char *file_path);

/** Copy files completely from scratch */
extern long copyFile (int source_fd, int destination_fd);

#endif