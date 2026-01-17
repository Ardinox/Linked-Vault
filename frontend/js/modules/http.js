import { Auth } from './auth.js';

export async function http(url, options = {}) {
  const token = Auth.getToken();

  if (!token) {
    return new Response(null, { status: 401 });
  }

  const headers = {
    'Content-Type': 'application/json',
    'Authorization': `Bearer ${token}`,
    ...options.headers
  };

  const response = await fetch(url, { ...options, headers });

  if (response.status === 401 || response.status === 403) {
    return response;
  }

  return response;
}
