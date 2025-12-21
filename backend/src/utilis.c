// File_Name utilis.c
// Utility Functions that helps main functions in their operations
// -- Dependecies --
// cJSON.h: Handles json

#include <ctype.h>

#include "cJSON.h"

#include "utilis.h"
#include "employee.h"

// ----HELPER: Validation Helper----
bool isOnlyAlphaSpaces(const char *str)
{
  for (int i = 0; str[i] != '\0'; i++)
  {

    if (!isalpha((unsigned char)str[i]) && str[i] != ' ')
    {
      return false;
    }
  }
  return true;
}

// --- HELPER: Recurrsive Json creator ---
void recursive_json_builder(emp *curr, cJSON *json_array)
{
  if (curr == NULL)
  {
    return;
  }

  // STEP 1. RECURSIVE CALL FIRST (Go to the end)
  recursive_json_builder(curr->next, json_array);

  // STEP 2. ADD TO JSON ON THE WAY BACK (Back-Tracking)
  cJSON *emp_obj = cJSON_CreateObject();
  cJSON_AddNumberToObject(emp_obj, "id", curr->id);
  cJSON_AddStringToObject(emp_obj, "name", curr->name);
  cJSON_AddNumberToObject(emp_obj, "age", curr->age);
  cJSON_AddStringToObject(emp_obj, "department", curr->department);
  cJSON_AddNumberToObject(emp_obj, "salary", curr->salary);

  // 3. ADD THE DATA OF EACH EMPLOYEE TO FULL TABLE
  cJSON_AddItemToArray(json_array, emp_obj);
}