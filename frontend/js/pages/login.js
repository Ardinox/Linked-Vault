import { CONFIG } from '../modules/config.js';

document.getElementById('loginForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const errorDiv = document.getElementById('error-msg');

    try {
        const response = await fetch(`${CONFIG.API_URL}/login`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
        });

        const data = await response.json();

        if (response.ok) {
            localStorage.setItem('authToken', data.token);
            // Redirect to ADMIN page
            window.location.href = 'admin.html'; 
        } else {
            errorDiv.textContent = data.message || "Login Failed";
            errorDiv.classList.remove('d-none');
        }
    } catch (err) {
        console.error(err);
        errorDiv.textContent = "Server Connection Error";
        errorDiv.classList.remove('d-none');
    }
});