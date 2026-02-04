const getApiUrl = () => {
    const host = window.location.hostname;

    // 1. DEVELOPMENT MODE
    if (host === '127.0.0.1' || host === 'localhost') {
        return "http://localhost:4000";
    }

    // 2. CLOUDFLARE / DOCKER (PRODUCTION MODE)
    return "/api";
};

export const CONFIG = {
    API_URL: getApiUrl()
};