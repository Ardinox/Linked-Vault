import { CONFIG } from '../modules/config.js';

document.getElementById('signupForm').addEventListener('submit', async (e) => {
    e.preventDefault();

    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const confirmPassword = document.getElementById('confirmPassword').value;
    
    const errorDiv = document.getElementById('error-msg');
    const successDiv = document.getElementById('success-msg');

    // Reset messages
    errorDiv.classList.add('d-none');
    successDiv.classList.add('d-none');

    // 1. Client-side Validation
    if (password !== confirmPassword) {
        errorDiv.textContent = "Passwords do not match!";
        errorDiv.classList.remove('d-none');
        return;
    }

    try {
        // 2. Call Node Gateway
        const response = await fetch(`${CONFIG.API_URL}/register`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
        });

        const data = await response.json();

        if (response.ok) {
            // 3. Success
            successDiv.textContent = "Account created! Redirecting to login...";
            successDiv.classList.remove('d-none');
            
            // Wait 1.5s then redirect
            setTimeout(() => {
                window.location.href = 'login.html';
            }, 1500);
        } else {
            // 4. Error from Server (e.g., Username taken)
            errorDiv.textContent = data.message || data.error || "Registration Failed";
            errorDiv.classList.remove('d-none');
        }
    } catch (err) {
        console.error(err);
        errorDiv.textContent = "Server Connection Error";
        errorDiv.classList.remove('d-none');
    }
});