// frontend/js/modules/auth.js

export const Auth = {
    getToken: () => localStorage.getItem('authToken'),
    
    // Check if user is logged in
    checkSession: () => {
        const token = localStorage.getItem('authToken');
        if (!token) {
            window.location.href = '../views/login.html';
            throw new Error("Halted: No Token");
        }
        return token;
    },

    // Handles Logout
    logout: () => {
        localStorage.removeItem('authToken');
        localStorage.removeItem('currentUser');
        alert("You are Logged out!");
        window.location.href = '../views/login.html';
    }
};