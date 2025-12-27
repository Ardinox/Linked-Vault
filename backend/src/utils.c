// File_Name utilis.c
// Utility Functions that helps main functions in their operations

#include "utils.h"
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

// --- HELPER: Check HTTP Method ---
int is_method(struct mg_http_message *hm, const char *method)
{
  if (hm->method.len != strlen(method))
  {
    return 0;
  }

  return memcmp(hm->method.buf, method, hm->method.len) == 0;
}

// --- HELPER:  Core Validations logic ---
const char *validate_core_logic(int id, char *name, int age, char *dept, int salary){

  // 1. Check Unique Id
  int new_id = id;
  emp *scanner = my_list.head;
  while (scanner != NULL)
  {
    if (scanner->id == new_id)
    {
      // ID FOUND! Return Error immediately.
      return "Employee ID already exists";
    }
    scanner = scanner->next;
  }

  // Check Name Format (Prevent special chars/scripts)
  if (!isOnlyAlphaSpaces(name))
  {
    return "Name must contain only alphabets and spaces";
  }

  // Check for available charector size for Name (Buffer Overflow Prevention)
  if (strlen(name) >= 50)
  {
    return "Name is too long! Max 49 characters allowed.";
  }

  // Check Department Format (Prevent special chars/scripts)
  if (!isOnlyAlphaSpaces(dept))
  {
    return "Department field should only contain alphabet and spaces!";
  }

  // Check for available character size for Department (Buffer Overflow Prevention)
  if (strlen(dept) >= 50)
  {
    return "Department is too long! Max 49 characters allowed.";
  }

  // Check for valid age
  if (age < 16 || age > 100)
  {
    return "Enter a valid Age!";
  }

  // Check for valid salary (salary can't be negative)
  if (salary < 0)
  {
    return "Enter a Valid Salary!";
  }

  // All checks Passed
  return NULL;
}

// --- HELPER: Validations for new insertions using json ---
const char *validate_employee_json(cJSON *j_data)
{
  if (!j_data)
  {
    return "Missing 'data' object";
  }

  // Extract Fields
  cJSON *j_id = cJSON_GetObjectItem(j_data, "id");
  cJSON *j_name = cJSON_GetObjectItem(j_data, "name");
  cJSON *j_age = cJSON_GetObjectItem(j_data, "age");
  cJSON *j_dept = cJSON_GetObjectItem(j_data, "department");
  cJSON *j_salary = cJSON_GetObjectItem(j_data, "salary");

  // 1. Check Missing Fields
  if (!j_id || !j_name || !j_age || !j_dept || !j_salary)
  {
    return "All fields are required";
  }

  // 2. If Strings are actually strings
  if(!cJSON_IsString(j_name) || !cJSON_IsString(j_dept)){
    return "Name and Department must be String";
  }

  // 3. Check Numbers are actually numbers
  if (!cJSON_IsNumber(j_id) || !cJSON_IsNumber(j_age) || !cJSON_IsNumber(j_salary))
  {

    return "ID, Age, and Salary must be numbers";
  }

  // 4. Core validations
  const char *error_msg = validate_core_logic(j_id->valueint, j_name->valuestring, j_age->valueint, j_dept->valuestring, j_salary->valueint);
  if(error_msg != NULL){
    return error_msg;
  }
  

  // return 'NULL' if no error found
  return NULL;
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

// --- HELPER: To add to the global list efficiently ---
void append_to_list(int id, char *name, int age, char *dept, int salary)
{
  emp *new_node = (emp *)malloc(sizeof(emp));

  // Set data
  new_node->id = id;

  strncpy(new_node->name, name, 49);
  new_node->name[49]='\0';

  new_node->age = age;

  strncpy(new_node->department, dept, 49);
  new_node->department[49]='\0';

  new_node->salary = salary;
  new_node->next = NULL;

  // Linking
  if (my_list.head == NULL)
  {
    my_list.head = new_node;
    my_list.tail = new_node;
  }
  else
  {
    my_list.tail->next = new_node;
    my_list.tail = new_node;
  }
}