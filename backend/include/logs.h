#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

void get_time_string(char *buffer, size_t size);

void add_log(int owner_id, const char *action, const char *details);


#endif