/**
 * FaarisMart API Configuration
 * Centralized API endpoint configuration for all frontend pages.
 *
 * TO CONNECT TO RENDER IN PRODUCTION:
 * Set PRODUCTION_API_URL below to your Render Web Service URL
 * Example: const PRODUCTION_API_URL = "https://faarismart-backend.onrender.com";
 *
 * OR in browser console run:
 * localStorage.setItem("FAARISMART_API_URL", "https://your-service.onrender.com");
 */
const PRODUCTION_API_URL = "";

function getApiBaseUrl() {
    // 1. Manually configured production URL
    if (PRODUCTION_API_URL && PRODUCTION_API_URL.trim() !== "") {
        return PRODUCTION_API_URL.trim().replace(/\/+$/, "");
    }

    // 2. Global window configuration override
    if (typeof window !== "undefined" && window.FAARISMART_API_URL) {
        return String(window.FAARISMART_API_URL).trim().replace(/\/+$/, "");
    }

    // 3. Dynamic browser localStorage configuration
    try {
        const stored = localStorage.getItem("FAARISMART_API_URL");
        if (stored && stored.trim() !== "") {
            return stored.trim().replace(/\/+$/, "");
        }
    } catch (e) {}

    // 4. Default development URL
    return "http://127.0.0.1:8080";
}

const API_URL = getApiBaseUrl();
const API = API_URL;

if (typeof window !== "undefined") {
    window.API_URL = API_URL;
    window.API = API;
    window.getApiBaseUrl = getApiBaseUrl;
}

async function apiGet(route) {
    const base = getApiBaseUrl();
    const res = await fetch(`${base}${route}`);
    if (!res.ok) {
        throw new Error(`GET ${route} failed with status ${res.status}`);
    }
    return await res.json();
}

async function apiPost(route, data) {
    const base = getApiBaseUrl();
    const res = await fetch(`${base}${route}`, {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(data)
    });
    return await res.json();
}