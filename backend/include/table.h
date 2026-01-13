#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "employee.h"

// Data Structure for Tables
typedef struct Table {
    char table_id[50];
    EmployeeList employeelist;
    struct Table *next;
    pthread_mutex_t lock;
} Table;

extern Table *global_tables_head;
extern pthread_mutex_t global_list_lock;

Table* get_or_create_table(const char *table_id);

#endif