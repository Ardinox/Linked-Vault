#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "cJSON.h"

// --- DATA STRUCTURES ---

// 1. Linked List Node (For RAM)
typedef struct employee
{
  int id;
  int age;
  int salary;
  char name[50];
  char department[50];
  struct employee *next;
} emp;

// 2. Serialization Struct (For Disk - No Pointers!)
// Fixed: Removed the "employee" tag here to prevent duplicate definition error
typedef struct 
{
  int id;
  int age;
  int salary;
  char name[50];
  char department[50];
} EmployeeRecord;

// 3. The List Wrapper
typedef struct
{
  emp *head;
  emp *tail;
} EmployeeList;

// --- FUNCTION DECLARATIONS ---
void init_employee_list(EmployeeList *list);
// Creates a node from JSON data
emp *create_node_from_json(cJSON *json);

#endif