#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "cJSON.h"
#include "mongoose.h"
#include "employee.h"
#include "table.h" 
// Note: table.h includes storage.h, so we have access to User structs if needed

// --- HELPER: String Validation ---
bool isOnlyAlphaSpaces(const char *str);

// --- HELPER: Check HTTP Method ---
int is_method(struct mg_http_message *hm, const char *method);

// --- HELPER: Core Validations logic ---
// Validates raw data before creating a node
const char *validate_core_logic(Table *t, int id, char *name, int age, char *dept, int salary);

// --- HELPER: Case-Insensitive String Contains ---
// Returns 1 if 'needle' is found inside 'haystack' (ignoring case), 0 otherwise
int str_contains_ci(const char *haystack, const char *needle);

// --- HELPER: JSON Validations ---
// Validates incoming JSON for employee creation
const char* validate_employee_json(cJSON *j_data, Table *t);

// --- HELPER: Recursive Reverse ---
// Helper for the recursive reverse JSON builder
void recursive_json_builder(emp *curr, cJSON *json_array);

// --- HELPER: Linked List Operations ---
// Add to end of list (RAM only)
void append_to_list(Table *t, int id, char *name, int age, char *dept, int salary);

// Insert at specific position (RAM only)
void insert_node_at_pos(Table *t, emp *insert, int position);

#endif