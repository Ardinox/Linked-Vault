import { Auth } from './auth.js';

export async function http(url, options = {}) {
  // 1. Get Token
  const token = Auth.getToken();

  // 2. If no token exists, use your Auth logic to halt & redirect
  if (!token) {
    try {
        Auth.checkSession(); // This will redirect and throw "Halted"
    } catch (e) {
        // We catch the error so it doesn't clutter the console, 
        // but we return a Promise that never resolves to stop the UI.
        return new Promise(() => {});
    }
  }

  // 3. Prepare Headers
  const headers = {
    'Content-Type': 'application/json',
    'Authorization': `Bearer ${token}`,
    ...options.headers
  };

  try {
      const response = await fetch(url, { ...options, headers });

      // 4. Server says Token Expired (401) or Forbidden (403)
      if (response.status === 401) {
        console.warn("Session expired (401). Logging out...");
        Auth.logout(); 
        return new Promise(() => {});
      }
      if (response.status === 403) {
        console.warn(`⛔ Access Denied (403) for ${url}`);
        throw new Error("Access Denied: You do not have permission to view this.");
      }

      return response;
  } catch (error) {
      console.error("Network Error:", error);
      throw error;
  }
}