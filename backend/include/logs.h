#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

// Helper: Get current Timestamp
void get_time_string(char *buffer, size_t size);

// 1. WRITE LOG (The "Add" Function)
void add_log(int owner_id, const char *action, const char *details);


#endif