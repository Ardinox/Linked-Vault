// frontend/js/pages/landing.js

document.addEventListener('DOMContentLoaded', () => {
    const btn = document.getElementById("getStartedBtn");
    
    if (btn) {
        btn.addEventListener("click", () => {
            const token = localStorage.getItem("authToken");
            
            if (token) {
                // User is already logged in -> Go to Admin Dashboard
                window.location.href = "views/admin.html";
            } else {
                // User is guest -> Go to Login
                window.location.href = "views/login.html";
            }
        });
    }
});