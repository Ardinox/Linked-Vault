#ifndef STORAGE_H
#define STORAGE_H

#include "table.h"

// --- Core Storage Functions ---

// Saves the entire linked list of a specific table to a binary file
void save_table_binary(Table *t);

// Loads data from a binary file into the table's linked list
void load_table_binary(Table *t);

// --- Helper Functions ---
EmployeeRecord node_to_record(emp *node);
emp *record_to_node(EmployeeRecord record);

#endif