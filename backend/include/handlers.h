#ifndef HANDLERS_H
#define HANDLERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mongoose.h"
#include "cJSON.h"

// --- DECLARATION OF AUTH FUNCTION ---
void handle_get_user(struct mg_connection *c, struct mg_http_message *hm);

void handle_registry(struct mg_connection *c, struct mg_http_message *hm);

// --- DECLARATION OF TABLE FUNCTION ---
void handle_create_table(struct mg_connection *c, struct mg_http_message *hm);

void handle_list_table(struct mg_connection *c, struct mg_http_message *hm);

void handle_get_tables(struct mg_connection *c, struct mg_http_message *hm);

// --- DECLARATION FOR CORE SERVER FUNCTIONS ---
// --- 1. Insertion ---
void handle_insertion(struct mg_connection *c, struct mg_http_message *hm);

// --- 2. Display ---
void handle_showall(struct mg_connection *c, struct mg_http_message *hm);

// --- 3. Searching ---
void handle_search_by_id(struct mg_connection *c, struct mg_http_message *hm);

// --- 4. Deletion ---
void handle_delete(struct mg_connection *c, struct mg_http_message *hm);

// --- 5. Linked List Reverse ---
void handle_reverse(struct mg_connection *c, struct mg_http_message *hm);

// --- 6. Recursive Reverse ---
void handle_recursive_reverse(struct mg_connection *c, struct mg_http_message *hm);

// --- Handle Update ---
void handle_update(struct mg_connection *c, struct mg_http_message *hm);

// --- 7. CSV Import ---
void handle_import(struct mg_connection *c, struct mg_http_message *hm);

// --- 8. CSV Export ---
void handle_export(struct mg_connection *c, struct mg_http_message *hm);

// --- 9. Linked List Cleanup ---
void handle_clear_linkedlist(struct mg_connection *c, struct mg_http_message *hm);

#endif