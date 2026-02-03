import { http } from "./http.js";
import { CONFIG } from "./config.js";

// Make sure this matches your Gateway URL
const BASE = CONFIG.API_URL; 

export const Api = {
  // --- TABLE MANAGEMENT ---
  async getTables() {
    const res = await http(`${BASE}/list_tables`, {
        method: "POST" 
    });
    return res.ok ? await res.json() : [];
  },

  async createTable(name) {
    return await http(`${BASE}/my_tables`, {
      method: "POST",
      body: JSON.stringify({ name }), 
    });
  },

  async clearTable(tableId) {
    return await http(`${BASE}/delete_table?table_id=${tableId}`, {
      method: "DELETE",
    });
  },

  // --- READ DATA ---
  async getAll(tableId) {
    const res = await http(`${BASE}/show?table_id=${tableId}`);
    return res.ok ? await res.json() : [];
  },

  // --- Search Employee ---
  async search(queryText, tableId) {
    const res = await http(`${BASE}/search?query=${encodeURIComponent(queryText)}&table_id=${tableId}`);
    return res.ok ? await res.json() : [];
  },

  // --- Insert Employee Details ---
  async insert(payload) {
    return await http(`${BASE}/insert`, {
      method: "POST",
      body: JSON.stringify(payload),
    });
  },

  // --- Update Employee Details ---
  async update(payload) {
    return await http(`${BASE}/update`, {
      method: "PUT",
      body: JSON.stringify(payload),
    });
  },

  // --- Delete Employee Details ---
  async delete(id, tableId) {
    return await http(`${BASE}/delete?id=${id}&table_id=${tableId}`, {
      method: "DELETE",
    });
  },

  // --- TOOLS ---
  async reverse(tableId) {
    return await http(`${BASE}/linkedreverse?table_id=${tableId}`, {
      method: "PUT",
    });
  },

  async recursiveReverse(tableId) {
    const res = await http(`${BASE}/recursivereverse?table_id=${tableId}`);
    return res.ok ? await res.json() : [];
  },

  // --- FILES ---
  getDownloadUrl(tableId) {
    const token = localStorage.getItem('authToken');
    // we handle the token manually in the UI for downloads if needed.
    return `${BASE}/download_table?table_id=${tableId}`;
  },

  async uploadCsv(tableId, csvContent) {
    return await http(`${BASE}/upload_csv?table_id=${tableId}`, {
      method: "POST",
      headers: { "Content-Type": "text/plain" },
      body: csvContent,
    });
  },
  async getHistory() {
    const res = await http(`${BASE}/api/history`);
    return res.ok ? await res.json() : [];
  },
};