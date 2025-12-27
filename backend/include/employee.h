#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

// --- A. DATA STRUCTURES ---
typedef struct employee
{
  int id;
  int age;
  int salary;
  char name[50];
  char department[50];
  struct employee *next;
} emp;

typedef struct
{
  emp *head;
  emp *tail;
} EmployeeList;

// Global list so data persists between requests
extern EmployeeList my_list;

// Delaration of node creation function
emp *create_node_from_json(cJSON *json);

#endif