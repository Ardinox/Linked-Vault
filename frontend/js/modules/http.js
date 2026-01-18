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
      if (response.status === 401 || response.status === 403) {
        console.warn("Server rejected session. Logging out...");
        Auth.logout(); // Triggers alert + redirect
        
        // Return a pending promise to freeze the UI while redirecting
        return new Promise(() => {}); 
      }

      return response;
  } catch (error) {
      console.error("Network Error:", error);
      throw error;
  }
}