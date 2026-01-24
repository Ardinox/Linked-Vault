#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "employee.h"
#include "storage.h"

// --- RUNTIME DATA STRUCTURE ---
// Represents a Table currently loaded in RAM
typedef struct Table {
    int id;                 // Matches TableMetadata.id
    int owner_id;           // Matches User.id
    char display_name[50];  // For display purposes
    
    EmployeeList employeelist; // The actual data
    
    struct Table *next;     // For the global linked list of loaded tables
    pthread_mutex_t lock;   // Concurrency lock
} Table;

extern Table *global_tables_head;
extern pthread_mutex_t global_list_lock;

// --- FUNCTIONS ---

// Load a table into RAM (or return existing). 
// Requires owner_id for security verification.
Table* get_or_load_table(int table_id, int owner_id);

// Unload a table from RAM (optional cleanup)
void unload_table(int table_id);

#endif