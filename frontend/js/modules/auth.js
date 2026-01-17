export const Auth = {
    getToken: () => localStorage.getItem('authToken'),
    
    // The logic from "3. AUTH CHECK"
    checkSession: () => {
        const token = localStorage.getItem('authToken');
        if (!token) {
            window.location.href = '../views/login.html';
            throw new Error("Halted: No Token");
        }
        return token;
    },

    logout: () => {
        alert("Session Expired. Please login again.");
        localStorage.removeItem('authToken');
        window.location.href = '../views/login.html';
    }
};