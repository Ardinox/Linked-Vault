#ifndef HANDLERS_H
#define HANDLERS_H

#include "mongoose.h"

// --- DECLARATION OF SERVER FUNCTIONS ---

// --- 1. Insertion ---
void handle_insertion(struct mg_connection *c, struct mg_http_message *hm);

// --- 2. Display ---
void handle_showall(struct mg_connection *c);

// --- 3. Searching ---
void handle_search_by_id(struct mg_connection *c, struct mg_http_message *hm);

// --- 4. Deletion ---
void handle_delete(struct mg_connection *c, struct mg_http_message *hm);

// --- 5. Linked List Reverse ---
void handle_reverse(struct mg_connection *c, struct mg_http_message *hm);

// --- 6. Recursive Reverse ---
void handle_recursive_reverse(struct mg_connection *c);

#endif