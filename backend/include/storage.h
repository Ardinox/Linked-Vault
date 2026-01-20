#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "employee.h" 

// --- DATA STRUCTURES (DATABASE MODELS) ---

typedef struct {
    int id;                 // Unique ID (1, 2, 3...)
    char username[50];      // "admin"
    char password_hash[128]; // The hash
} User;

typedef struct {
    int id;                 // The real file ID (e.g., 500 -> "bin/table_500.bin")
    int owner_id;           // Foreign Key: Who owns this? (Links to User.id)
    char table_name[50];    // Friendly Name: "My Project 2026"
    int is_active;          // 1 = Active, 0 = Deleted (Soft Delete)
} TableMetadata;

// Forward declaration
struct Table; 

// --- HELPER FUNCTIONS ---
EmployeeRecord node_to_record(emp *node);
emp *record_to_node(EmployeeRecord record);

// --- CORE STORAGE FUNCTIONS ---
void save_table_binary(struct Table *t);
void load_table_binary(struct Table *t);

// --- USER AUTH FUNCTIONS ---
// (Renamed for consistency)
User find_user_by_name(const char* username);
int save_new_user(const char* username, const char *hash); // Corresponds to register_user

// --- TABLE REGISTRY FUNCTIONS ---
// Creates a new table entry. Returns new table_id.
int save_table_metadata(int owner_id, const char* display_name);
TableMetadata* get_user_tables(int user_id, int *count);

// Verify if a table belongs to a user (Security Check)
int is_table_owner(int user_id, int table_id);

#endif