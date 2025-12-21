// File_Name main.c
// Entry Point for the LinkedVault Backend Server

#include "handlers.h"
#include "employee.h"
#include "utilis.h"

// --- CONSTANTS ---
// Listening on 0.0.0.0 allows access from external IPs, not just localhost.
static const char *LISTENING_ADDR = "http://0.0.0.0:8000";

// The poll interval (1000ms) determines how long the call blocks if there are no events.
static const int POLL_INTERVAL_MS = 1000;

// ---  EVENT HANDLER ---
static void fn(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_HTTP_MSG)
  {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    /* * --- CORS PREFLIGHT HANDLER ---
     * Browsers send an OPTIONS request before POST requests to check permissions.
     * We must reply with Access-Control-Allow-* headers, or the browser will block
     * the subsequent POST request.
     */
    if (hm->method.len == 7 && memcmp(hm->method.buf, "OPTIONS", 7) == 0)
    {
      mg_http_reply(c, 200,
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type\r\n",
                    "");
      return;
    }

    //  --- ROUTING LOGIC ---
    if (mg_match(hm->uri, mg_str("/insert"), NULL))
    {
      handle_insertion(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/show"), NULL))
    {
      handle_showall(c);
    }
    else if (mg_match(hm->uri, mg_str("/search"), NULL))
    {
      handle_search_by_id(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/delete"), NULL))
    {
      handle_delete(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/linkedreverse"), NULL))
    {
      handle_reverse(c, hm);
    }
    else if (mg_match(hm->uri, mg_str("/recursivereverse"), NULL))
    {
      handle_recursive_reverse(c);
    }
    // Default Catch-All: 404 Not Found
    else
    {
      mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\n",
                    "Use POST /insert, /show, /delete, or /search\n");
    }
  }
}

int main(void)
{
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);

  // Setup the HTTP listener.
  mg_http_listen(&mgr, LISTENING_ADDR, fn, NULL);
  printf("Server running on port 8000...\n");

  // --- Main Loop ---
  // This Infinite loop keep the server running
  for (;;)
  {
    mg_mgr_poll(&mgr, POLL_INTERVAL_MS);
  }

  // Cleanup (Good Practice)
  mg_mgr_free(&mgr);
  return 0;
}