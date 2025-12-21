#ifndef UTILIS_H
#define UTILIS_H

#include <stdbool.h>
#include "cJSON.h"
#include "employee.h"

// --- DECLARATION OF HELPER FUNCTION ---
// --- Helper for String Validation ---
bool isOnlyAlphaSpaces(const char *str);

// --- Helper for Recursive Reverse Function ---
void recursive_json_builder(emp *curr, cJSON *json_array);

#endif