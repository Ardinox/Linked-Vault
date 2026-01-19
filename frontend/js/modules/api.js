import { http } from "./http.js";

// Make sure this matches your Gateway URL
const BASE = "http://localhost:4000"; 

export const Api = {
  // --- GET DATA ---
  async getAll(tableId) {
    const res = await http(`${BASE}/show?table_id=${tableId}`);
    return res.ok ? await res.json() : [];
  },

  async getTables() {
    const res = await http(`${BASE}/my_tables`);
    return res.ok ? await res.json() : [];
  },

  async search(id, tableId) {
    const res = await http(`${BASE}/search?id=${id}&table_id=${tableId}`);
    if (res.status === 404) return null;
    return res.ok ? await res.json() : null;
  },

  // --- WRITE DATA ---
  async createTable(tableName) {
    return await http(`${BASE}/my_tables`, {
      method: "POST",
      body: JSON.stringify({ tableName }),
    });
  },

  async insert(payload) {
    return await http(`${BASE}/insert`, {
      method: "POST",
      body: JSON.stringify(payload),
    });
  },

  async update(payload) {
    return await http(`${BASE}/update`, {
      method: "PUT",
      body: JSON.stringify(payload),
    });
  },

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

  async clearTable(tableId) {
    return await http(`${BASE}/clear_table?table_id=${tableId}`, {
      method: "DELETE",
    });
  },

  // --- FILES ---
  getDownloadUrl(tableId) {
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
};