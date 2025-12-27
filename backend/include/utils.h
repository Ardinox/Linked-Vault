// -- Dependecies --
// cJSON.h: Handles json
// mongoose.h: handles the web server

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

// --- DECLARATION OF HELPER FUNCTION ---
// --- Helper for String Validation ---
bool isOnlyAlphaSpaces(const char *str);

// --- HELPER: Check HTTP Method ---
int is_method(struct mg_http_message *hm, const char *method);

// --- HELPER:  Core Validations logic ---
const char *validate_core_logic(int id, char *name, int age, char *dept, int salary);

// --- HELPER: Validations for new insertions using json ---
const char* validate_employee_json(cJSON *j_data);

// --- Helper for Recursive Reverse Function ---
void recursive_json_builder(emp *curr, cJSON *json_array);

// --- HELPER: To add to the global list efficiently ---
void append_to_list(int id, char *name, int age, char *dept, int salary);

#endif